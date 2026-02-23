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
	}

	return Matrix4x4();
}

ST_BoundingBox BoundingBoxTransform(ST_BoundingBox const& b, Matrix4x4 const& m)
{
	return ST_BoundingBox();
}

NAMESPACE_XYH_END
