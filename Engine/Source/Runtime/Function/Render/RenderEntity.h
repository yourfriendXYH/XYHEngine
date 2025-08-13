#pragma once
#include <Common.h>
#include <vector>
#include "Runtime/Core/Math/Matrix4.h"
#include "Runtime/Core/Math/AxisAligned.h"

NAMESPACE_XYH_BEGIN

// 渲染实体类
class RenderEntity
{
public:
    uint32_t  m_instanceId = 0u;
    Matrix4x4 m_modelMatrix = Matrix4x4::IDENTITY;

    // mesh
	size_t m_meshAssetId = 0u;  // 网格资源ID
    bool m_enableVertexBlending = false;
	std::vector<Matrix4x4> m_jointMatrices; // 关节矩阵
	AxisAlignedBox m_boundingBox;   // 包围盒

    // material
	size_t m_materialAssetId = 0u; // 材质资源ID
	bool m_blend = false;   // 是否启用混合
	bool m_doubleSided = false; // 是否双面渲染
	Vector4 m_baseColor_Factor = { 1.0f, 1.0f, 1.0f, 1.0f };    // 基础颜色因子
	float m_metallicFactor = 1.0f; // 金属度因子
	float m_roughnessFactor = 1.0f; // 粗糙度因子
	float m_normalScale = 1.0f;	// 法线贴图缩放因子
	float m_occlusionStrength = 1.0f;	// 遮挡强度
	Vector3 m_emissiveFactor = { 0.0f, 0.0f, 0.0f };	// 自发光因子
};

NAMESPACE_XYH_END