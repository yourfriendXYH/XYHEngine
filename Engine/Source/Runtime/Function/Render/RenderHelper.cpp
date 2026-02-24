#include "RenderHelper.h"
#include <Runtime/Core/Math/Matrix4.h>
#include "RenderScene.h"
#include "RenderCamera.h"

NAMESPACE_XYH_BEGIN

Matrix4x4 CalculateDirectionalLightCamera(RenderScene& scene, RenderCamera& camera)
{
	Matrix4x4 projViewMatrix;
	{
		Matrix4x4 viewMatrix = camera.GetViewMatrix();
		Matrix4x4 projMatrix = camera.GetPersProjMatrix();
		projViewMatrix = projMatrix * viewMatrix;
	}

	// 相机视锥体的包围盒
	ST_BoundingBox frustumBoundingBox;	// 视锥体Box，世界空间下的BoundingBox
	{
		Vector3 const g_frustumPointsNDCSpace[8] = {
			Vector3(-1.0f, -1.0f, 1.0f),
			Vector3(1.0f, -1.0f, 1.0f),
			Vector3(1.0f, 1.0f, 1.0f),
			Vector3(-1.0f, 1.0f, 1.0f),
			Vector3(-1.0f, -1.0f, 0.0f),
			Vector3(1.0f, -1.0f, 0.0f),
			Vector3(1.0f, 1.0f, 0.0f),
			Vector3(-1.0f, 1.0f, 0.0f)
		};

		Matrix4x4 inverseProjViewMatrix = projViewMatrix.inverse();

		frustumBoundingBox.m_minBound = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		frustumBoundingBox.m_maxBound = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);

		size_t const cornerCount = 8u;
		for (size_t i = 0; i < cornerCount; ++i)
		{
			Vector4 frustumPointWithW = inverseProjViewMatrix * Vector4(g_frustumPointsNDCSpace[i].x, g_frustumPointsNDCSpace[i].y, g_frustumPointsNDCSpace[i].z, 1.0);
			Vector3 frustumPoint = Vector3(frustumPointWithW.x / frustumPointWithW.w, frustumPointWithW.y / frustumPointWithW.w, frustumPointWithW.z / frustumPointWithW.w);
			frustumBoundingBox.Merge(frustumPoint);
		}

	}

	// 场景的包围盒
	ST_BoundingBox sceneBoundingBox;
	{
		sceneBoundingBox.m_minBound = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		sceneBoundingBox.m_maxBound = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);

		for (const RenderEntity& entity : scene.m_renderEntities)
		{
			// 网格的BoundingBox
			ST_BoundingBox meshAssetBoundingBox{ entity.m_boundingBox.GetMinCorner(), entity.m_boundingBox.GetMaxCorner() };
			ST_BoundingBox meshBoundingBoxWorld = BoundingBoxTransform(meshAssetBoundingBox, entity.m_modelMatrix);

			sceneBoundingBox.Merge(meshBoundingBoxWorld);
		}
	}

	// 直射光的矩阵
	Matrix4x4 lightView;
	Matrix4x4 lightProj;
	{
		Vector3 boxCenter(
			(frustumBoundingBox.m_maxBound.x + frustumBoundingBox.m_minBound.x) * 0.5,
			(frustumBoundingBox.m_maxBound.y + frustumBoundingBox.m_minBound.y) * 0.5,
			(frustumBoundingBox.m_maxBound.z + frustumBoundingBox.m_minBound.z)
		);

		Vector3 boxExtents(
			(frustumBoundingBox.m_maxBound.x - frustumBoundingBox.m_minBound.x) * 0.5,
			(frustumBoundingBox.m_maxBound.y - frustumBoundingBox.m_minBound.y) * 0.5,
			(frustumBoundingBox.m_maxBound.z - frustumBoundingBox.m_minBound.z) * 0.5
		);

		Vector3 eye = boxCenter + scene.m_directionalLight.m_direction * boxExtents.length();
		Vector3 center = boxCenter;

		// 相机矩阵
		lightView = Math::makeLookAtMatrix(eye, center, Vector3(0.0f, 0.0f, 1.0f));	// 直射光的相机矩阵

		ST_BoundingBox frustumBoundingBoxLightView = BoundingBoxTransform(frustumBoundingBox, lightView);
		ST_BoundingBox sceneBoundingBoxLightView = BoundingBoxTransform(sceneBoundingBox, lightView);

		// 投影矩阵
		lightProj = Math::makeOrthographicProjectionMatrix01(
			std::max(frustumBoundingBoxLightView.m_minBound.x, sceneBoundingBoxLightView.m_minBound.x),	// left
			std::min(frustumBoundingBoxLightView.m_maxBound.x, sceneBoundingBoxLightView.m_maxBound.x),	// right
			std::max(frustumBoundingBoxLightView.m_minBound.y, sceneBoundingBoxLightView.m_minBound.y),	// bottom
			std::min(frustumBoundingBoxLightView.m_maxBound.y, sceneBoundingBoxLightView.m_maxBound.y),	// top
			// the objects which are nearer than the frustum bounding box may caster shadow as well
			-sceneBoundingBoxLightView.m_maxBound.z, // near
			-std::max(frustumBoundingBoxLightView.m_minBound.z, sceneBoundingBoxLightView.m_minBound.z)); // far

	}

	Matrix4x4 lightProjView = lightProj * lightView;
	return lightProjView;
}

ST_BoundingBox BoundingBoxTransform(ST_BoundingBox const& b, Matrix4x4 const& transformMatrix)
{
	Vector3 const g_BoxOffset[8] = { Vector3(-1.0f, -1.0f, 1.0f),
								  Vector3(1.0f, -1.0f, 1.0f),
								  Vector3(1.0f, 1.0f, 1.0f),
								  Vector3(-1.0f, 1.0f, 1.0f),
								  Vector3(-1.0f, -1.0f, -1.0f),
								  Vector3(1.0f, -1.0f, -1.0f),
								  Vector3(1.0f, 1.0f, -1.0f),
								  Vector3(-1.0f, 1.0f, -1.0f) };

	size_t const cornerCount = 8u;

	Vector3 center(
		(b.m_maxBound.x + b.m_minBound.x) * 0.5f,
		(b.m_maxBound.y + b.m_minBound.y) * 0.5f,
		(b.m_maxBound.z + b.m_minBound.z) * 0.5f
	);

	Vector3 extents(
		(b.m_maxBound.x - b.m_minBound.x) * 0.5f,
		(b.m_maxBound.y - b.m_minBound.y) * 0.5f,
		(b.m_maxBound.z - b.m_minBound.z) * 0.5f
	);

	Vector3 min;
	Vector3 max;
	for (size_t i = 0u; i < cornerCount; ++i)
	{
		Vector3 cornerBefore = extents * g_BoxOffset[i] + center;
		Vector4 cornerWithW = transformMatrix * Vector4(cornerBefore.x, cornerBefore.y, cornerBefore.z, 1.0);
		Vector3 corner = Vector3(cornerWithW.x / cornerWithW.w, cornerWithW.y / cornerWithW.w, cornerWithW.z / cornerWithW.w);

		if (i == 0u)
		{
			min = corner;
			max = corner;
		}
		else
		{
			min = Vector3(Math::min(min[0], corner[0]), Math::min(min[1], corner[1]), Math::min(min[2], corner[2]));
			max = Vector3(Math::max(max[0], corner[0]), Math::max(max[1], corner[1]), Math::max(max[2], corner[2]));
		}
	}

	ST_BoundingBox boundingBox(min, max);
	return boundingBox;
}

ST_ClusterFrustum CreateClusterFrustumFromMatrix(Matrix4x4 mat, float xLeft, float xRight, float yTop, float yBottom, float zNear, float zFar)
{
	ST_ClusterFrustum f;

	// the following is in the vulkan space
	// note that the Y axis is flipped in Vulkan
	assert(yTop < yBottom);

	assert(xLeft < xRight);
	assert(zNear < zFar);

	// calculate the tiled frustum
	// [Fast Extraction of Viewing Frustum Planes from the WorldView - Projection
	// Matrix](http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)

	Matrix4x4 mat_column = mat;

	// [vec.xyz 1][mat.col0] / [vec.xyz 1][mat.col3] > x_right
	// [vec.xyz 1][mat.col0 - mat.col3*x_right] > 0
	f.m_planeRight = Vector4(mat_column[0]) - (Vector4(mat_column[3]) * xRight);
	// normalize
	f.m_planeRight *= (1.0 / Vector3(f.m_planeRight.x, f.m_planeRight.y, f.m_planeRight.z).length());

	// for example, we try to calculate the "plane_left" of the tile frustum
	// note that we use the row vector to be consistent with the DirectXMath
	// [vec.xyz 1][mat.col0] / [vec.xyz 1][mat.col3] < x_left
	//
	// evidently, the value "[vec.xyz 1][mat.col3]" is expected to be greater than
	// 0 (w of clip space) and we multiply both sides by "[vec.xyz 1][mat.col3]",
	// the inequality symbol remains the same [vec.xyz 1][mat.col0] < [vec.xyz
	// 1][mat.col3]*x_left
	//
	// since "x_left" is a scalar, the "scalar multiplication" is applied
	// [vec.xyz 1][mat.col0] < [vec.xyz 1][mat.col3*x_left]
	// [vec.xyz 1][mat.col0 - mat.col3*x_left] < 0
	//
	// we follow the "DirectX::BoundingFrustum::Intersects", the normal of the
	// plane is pointing ourward [vec.xyz 1][mat.col3*x_left - mat.col0] > 0
	//
	// the plane can be defined as [x y z 1][A B C D] = 0 and the [A B C D] is
	// exactly [mat.col0 - mat.col3*x_left] and we need to normalize the normal[A
	// B C] of the plane let [A B C D] = [mat.col3*x_left - mat.col0] [A B C D] /=
	// length([A B C].xyz)
	f.m_planeLeft = (Vector4(mat_column[3]) * xLeft) - Vector4(mat_column[0]);
	// normalize
	f.m_planeLeft *= (1.0 / Vector3(f.m_planeLeft.x, f.m_planeLeft.y, f.m_planeLeft.z).length());

	// [vec.xyz 1][mat.col1] / [vec.xyz 1][mat.col3] < y_top
	// [vec.xyz 1][mat.col3*y_top - mat.col1] > 0
	f.m_planeTop = (Vector4(mat_column[3]) * yTop) - Vector4(mat_column[1]);
	// normalize
	f.m_planeTop *= (1.0 / Vector3(f.m_planeTop.x, f.m_planeTop.y, f.m_planeTop.z).length());

	// [vec.xyz 1][mat.col1] / [vec.xyz 1][mat.col3] > y_bottom
	// [vec.xyz 1][mat.col1 - mat.col3*y_bottom] > 0
	f.m_planeBottom = Vector4(mat_column[1]) - (Vector4(mat_column[3]) * yBottom);
	// normalize
	f.m_planeBottom *= (1.0 / Vector3(f.m_planeBottom.x, f.m_planeBottom.y, f.m_planeBottom.z).length());

	// [vec.xyz 1][mat.col2] / [vec.xyz 1][mat.col3] < z_near
	// [vec.xyz 1][mat.col3*z_near - mat.col2] > 0
	f.m_planeNear = (Vector4(mat_column[3]) * zNear) - Vector4(mat_column[2]);
	f.m_planeNear *= (1.0 / Vector3(f.m_planeNear.x, f.m_planeNear.y, f.m_planeNear.z).length());

	// [vec.xyz 1][mat.col2] / [vec.xyz 1][mat.col3] > z_far
	// [vec.xyz 1][mat.col2 - mat.col3*z_far] > 0
	f.m_planeFar = Vector4(mat_column[2]) - (Vector4(mat_column[3]) * zFar);
	f.m_planeFar *= (1.0 / Vector3(f.m_planeFar.x, f.m_planeFar.y, f.m_planeFar.z).length());

	return f;
}

bool TiledFrustumIntersectBox(ST_ClusterFrustum const& frustum, ST_BoundingBox const& boundingBox)
{
	// we follow the "DirectX::BoundingFrustum::Intersects"

	// Center of the box.
	Vector4 boxCenter(
		(boundingBox.m_maxBound.x + boundingBox.m_minBound.x) * 0.5,
		(boundingBox.m_maxBound.y + boundingBox.m_minBound.y) * 0.5,
		(boundingBox.m_maxBound.z + boundingBox.m_minBound.z) * 0.5,
		1.0);

	// Distance from the center to each side.
	// half extent //more exactly
	Vector3 boxExtents(
		(boundingBox.m_maxBound.x - boundingBox.m_minBound.x) * 0.5,
		(boundingBox.m_maxBound.y - boundingBox.m_minBound.y) * 0.5,
		(boundingBox.m_maxBound.z - boundingBox.m_minBound.z) * 0.5
	);

	// plane_right
	{
		float signedDistanceFromPlaneRight = frustum.m_planeRight.dotProduct(boxCenter);
		float radiusProjectPlaneRight = Vector3(fabs(frustum.m_planeRight.x), fabs(frustum.m_planeRight.y), fabs(frustum.m_planeRight.z)).dotProduct(boxExtents);

		bool intersectingOrInsideRight = signedDistanceFromPlaneRight < radiusProjectPlaneRight;
		if (!intersectingOrInsideRight)
		{
			return false;
		}
	}

	// plane_left
	{
		float signedDistanceFromPlaneLeft = frustum.m_planeLeft.dotProduct(boxCenter);
		float radiusProjectPlaneLeft = Vector3(fabs(frustum.m_planeLeft.x), fabs(frustum.m_planeLeft.y), fabs(frustum.m_planeLeft.z)).dotProduct(boxExtents);

		bool intersectingOrInsideLeft = signedDistanceFromPlaneLeft < radiusProjectPlaneLeft;
		if (!intersectingOrInsideLeft)
		{
			return false;
		}
	}

	// plane_top
	{
		float signedDistanceFromPlaneTop = frustum.m_planeTop.dotProduct(boxCenter);
		float radiusProjectPlaneTop = Vector3(fabs(frustum.m_planeTop.x), fabs(frustum.m_planeTop.y), fabs(frustum.m_planeTop.z)).dotProduct(boxExtents);

		bool intersectingOrInsideTop = signedDistanceFromPlaneTop < radiusProjectPlaneTop;
		if (!intersectingOrInsideTop)
		{
			return false;
		}
	}

	// plane_bottom
	{
		float signedDistanceFromPlaneBottom = frustum.m_planeBottom.dotProduct(boxCenter);
		float radiusProjectPlaneBottom = Vector3(fabs(frustum.m_planeBottom.x), fabs(frustum.m_planeBottom.y), fabs(frustum.m_planeBottom.z)).dotProduct(boxExtents);

		bool intersectingOrInsideBottom = signedDistanceFromPlaneBottom < radiusProjectPlaneBottom;
		if (!intersectingOrInsideBottom)
		{
			return false;
		}
	}

	// plane_near
	{
		float signedDistanceFromPlaneNear = frustum.m_planeNear.dotProduct(boxCenter);
		float radiusProjectPlaneNear = Vector3(fabs(frustum.m_planeNear.x), fabs(frustum.m_planeNear.y), fabs(frustum.m_planeNear.z)).dotProduct(boxExtents);

		bool intersectingOrInsideNear = signedDistanceFromPlaneNear < radiusProjectPlaneNear;
		if (!intersectingOrInsideNear)
		{
			return false;
		}
	}

	// plane_far
	{
		float signedDistanceFromPlaneFar = frustum.m_planeFar.dotProduct(boxCenter);
		float radiusProjectPlaneFar = Vector3(fabs(frustum.m_planeFar.x), fabs(frustum.m_planeFar.y), fabs(frustum.m_planeFar.z)).dotProduct(boxExtents);

		bool intersectingOrInsideFar = signedDistanceFromPlaneFar < radiusProjectPlaneFar;
		if (!intersectingOrInsideFar)
		{
			return false;
		}
	}

	return true;
}

bool BoxIntersectsWithSphere(ST_BoundingBox const& boundingBox, ST_BoundingSphere const& boundingSphere)
{
	for (size_t i = 0; i < 3; ++i)
	{
		if (boundingSphere.m_center[i] < boundingBox.m_minBound[i])
		{
			if ((boundingBox.m_minBound[i] - boundingSphere.m_center[i]) > boundingSphere.m_radius)
			{
				return false;
			}
		}
		else if (boundingSphere.m_center[i] > boundingBox.m_maxBound[i])
		{
			if ((boundingSphere.m_center[i] - boundingBox.m_maxBound[i]) > boundingSphere.m_radius)
			{
				return false;
			}
		}
	}

	return true;
}

NAMESPACE_XYH_END
