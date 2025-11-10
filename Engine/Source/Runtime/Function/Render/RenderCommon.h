#pragma once
#include <Common.h>
#include <Runtime/Core/Math/Matrix4.h>
#include "RenderType.h"
#include "Interface/RHI.h"

NAMESPACE_XYH_BEGIN

struct ST_VulkanMesh
{
	bool m_enableVertexBlending;    // 是否启用顶点混合

	// 顶点缓冲区
	uint32_t m_meshVertexCount; // 网格顶点数量
	RHIBuffer* m_meshVertexPositionBuffer;  // 网格顶点位置
	VmaAllocation m_meshVertexPositionBufferAllocation;

	RHIBuffer* m_meshVertexVaryingEnableBlendingBuffer; // 法线和切线方向
	VmaAllocation m_meshVertexVaryingEnableBlendingBufferAllocation;

	RHIBuffer* m_meshVertexJointBindingBuffer;  // 网格顶点关节绑定缓冲区
	VmaAllocation m_meshVertexJointBindingBufferAllocation;

	RHIDescriptorSet* m_meshVertexBlendingDescriptorSet;    // 网格顶点描述符集

	RHIBuffer* m_meshVertexVaryingBuffer;   // 纹理坐标
	VmaAllocation m_meshVertexVaryingBufferAllocation;

	// 索引缓冲区
	uint32_t m_meshIndexCount;
	RHIBuffer* m_meshIndexBuffer;
	VmaAllocation m_meshIndexBufferAllocation;
};

// material
struct ST_VulkanPBRMaterial
{
	RHIImage* m_baseColorTextureImage;  // 基础颜色纹理图像
	RHIImageView* m_baseColorImageView;
	VmaAllocation m_baseColorImageAllocation;

	RHIImage* m_metallicRoughnessTextureImage;  // 金属度和粗糙度纹理图像
	RHIImageView* m_metallicRoughnessImageView;
	VmaAllocation m_metallicRoughnessImageAllocation;

	RHIImage* m_normalTextureImage;	// 法线纹理图像
	RHIImageView* m_normalImageView;
	VmaAllocation m_normalImageAllocation;

	RHIImage* m_occlusionTextureImage;  // 遮挡纹理图像
	RHIImageView* m_occlusionImageView;
	VmaAllocation m_occlusionImageAllocation;

	RHIImage* m_emissiveTextureImage;   // 自发光纹理图像
	RHIImageView* m_emissiveImageView;
	VmaAllocation m_emissiveImageAllocation;

	RHIBuffer* m_materialUniformBuffer; // 材质统一缓冲区
	VmaAllocation m_materialUniformBufferAllocation;

	RHIDescriptorSet* m_materialDescriptorSet;  // 材质描述符集
};

struct ST_RenderMeshNode
{
	const Matrix4x4* m_modelMatrix = nullptr;  // 模型矩阵
	const Matrix4x4* m_jointMatrices = nullptr;    // 关节矩阵
	uint32_t m_jointCount = 0u; // 关节数量
	ST_VulkanMesh* m_refMesh = nullptr;	// 网格数据
	ST_VulkanPBRMaterial* m_refMaterial = nullptr;	// 材质数据
	uint32_t m_nodeId;
	bool m_enableVertexBlending = false;	// 是否启用顶点混合
};

struct ST_RenderAxisNode
{
	Matrix4x4 m_modelMatrix = Matrix4x4::IDENTITY;
	ST_VulkanMesh* m_refMesh = nullptr;
	uint32_t m_nodeId;
	bool m_enableVertexBlending = false;
};

static uint32_t const s_maxPointLightCount = 15u;
static uint32_t const s_meshPerDrawcallMaxInstanceCount = 64;
static uint32_t const s_meshVertexBlendingMaxJointCount = 1024; // 每个网格顶点混合的最大关节数量

struct ST_VulkanSceneDirectionalLight
{
	Vector3 m_direction;
	float paddingDirection;
	Vector3 m_color;
	float paddingColor;
};

struct ST_VulkanScenePointLight
{
	Vector3 m_position;
	float m_radius;
	Vector3 m_intensity;
	float paddingIntensity;
};

struct ST_MeshPerframeStorageBufferObject
{
	Matrix4x4 m_projViewMatrix;
	Vector3 m_cameraPosition;
	float paddingCameraPosition;
	Vector3 m_ambientLight;
	float paddingAmbientLight;
	uint32_t m_pointLightNum;
	uint32_t m_paddingPointLightNum1;
	uint32_t m_paddingPointLightNum2;
	uint32_t m_paddingPointLightNum3;
	ST_VulkanScenePointLight m_scenePointLights[s_maxPointLightCount];
	ST_VulkanSceneDirectionalLight m_sceneDirectionalLight;
	Matrix4x4 m_directionalLightProjView;
};

struct ST_VulkanMeshInstance
{
	float m_enableVertexBlending;
	float paddingEnableVertexBlending1;
	float paddingEnableVertexBlending2;
	float paddingEnableVertexBlending3;
	Matrix4x4 m_modelMatrix;
};

// 每个网格每次绘制调用的存储缓冲区对象
struct ST_MeshPerdrawcallStorageBufferObject
{
	ST_VulkanMeshInstance m_meshInstances[s_meshPerDrawcallMaxInstanceCount];
};

// 每个网格顶点混合的存储缓冲区对象
struct ST_MeshPerdrawcallVertexBlendingStorageBufferObject
{
	Matrix4x4 m_jointMatrices[s_meshVertexBlendingMaxJointCount * s_meshPerDrawcallMaxInstanceCount];
};

struct ST_AxisStorageBufferObject
{
	Matrix4x4 m_modelMatrix = Matrix4x4::IDENTITY;
	uint32_t m_selectedAxis = 3;
};

NAMESPACE_XYH_END