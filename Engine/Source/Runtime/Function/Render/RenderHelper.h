#pragma once
#include <Common.h>
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

NAMESPACE_XYH_BEGIN

class RenderScene;
class RenderCamera;

// 向上对齐
static inline uint32_t RoundUp(uint32_t value, uint32_t alignment)
{
	uint32_t temp = value + alignment - static_cast<uint32_t>(1);
	return (temp - temp % alignment);
}

struct ST_ClusterFrustum
{
	Vector4 m_planeRight;
	Vector4 m_planeLeft;
	Vector4 m_planeTop;
	Vector4 m_planeBottom;
	Vector4 m_planeNear;
	Vector4 m_planeFar;
};

struct ST_BoundingBox
{
public:
	ST_BoundingBox() {}

	ST_BoundingBox(const Vector3& minVec, const Vector3& maxVec)
	{
		m_minBound = minVec;
		m_maxBound = maxVec;
	}

	void Merge(const ST_BoundingBox& rhs)
	{
		m_minBound.makeFloor(rhs.m_minBound);
		m_maxBound.makeCeil(rhs.m_maxBound);
	}

	void Merge(const Vector3& point)
	{
		m_minBound.makeFloor(point);
		m_maxBound.makeCeil(point);
	}

public:
	Vector3 m_minBound{ std::numeric_limits<float>::max(),
					   std::numeric_limits<float>::max(),
					   std::numeric_limits<float>::max() };
	Vector3 m_maxBound{ std::numeric_limits<float>::min(),
					   std::numeric_limits<float>::min(),
					   std::numeric_limits<float>::min() };
};

// 球体包围盒
struct ST_BoundingSphere
{
	Vector3 m_center;
	float m_radius;
};

// 计算直射光的相机投影矩阵
Matrix4x4 CalculateDirectionalLightCamera(RenderScene& scene, RenderCamera& camera);

ST_BoundingBox BoundingBoxTransform(ST_BoundingBox const& b, Matrix4x4 const& transformMatrix);

ST_ClusterFrustum CreateClusterFrustumFromMatrix(Matrix4x4 mat, float xLeft, float xRight, float yTop, float yBottom, float zNear, float zFar);

// 视锥体求交，判断包围盒是否在视锥体可见范围内
bool TiledFrustumIntersectBox(ST_ClusterFrustum const& frustum, ST_BoundingBox const& boundingBox);

// 判断是否与球体包围盒相交
bool BoxIntersectsWithSphere(ST_BoundingBox const& boundingBox, ST_BoundingSphere const& boundingSphere);

NAMESPACE_XYH_END