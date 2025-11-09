#pragma once
#include <Common.h>
#include <vk_mem_alloc.h>
#include "Interface/RHI.h"
#include "RenderResourceBase.h"

NAMESPACE_XYH_BEGIN

// IBL（Image-Based Lighting，基于图像的照明）是一种先进的全局照明技术，
// 它使用环境贴图（通常是立方体贴图或等距柱状投影图）来模拟复杂的环境光照。
struct ST_IBLResource
{
	// BRDF LUT 纹理
	RHIImage* m_pBrdfLUTTextureImage;
	RHIImageView* m_pBrdfLUTTextureImageView;
	RHISampler* m_pBrdfLUTTextureSampler;
	VmaAllocation m_brdfLUTTextureImageAllocation;

	// 环境贴图 纹理
	RHIImage* m_pIrradianceTextureImage;
	RHIImageView* m_pIrradianceTextureImageView;
	RHISampler* m_pIrradianceTextureSampler;
	VmaAllocation m_irradianceTextureImageAllocation;

	// 反射贴图 纹理
	RHIImage* m_pSpecularTextureImage;
	RHIImageView* m_pSpecularTextureImageView;
	RHISampler* m_pSpecularTextureSampler;
	VmaAllocation m_specularTextureImageAllocation;
};

struct ST_ColorGradingResource
{
	RHIImage* m_pColorGradingLUTTextureImage;
	RHIImageView* m_pColorGradingLUTTextureImageView;
	VmaAllocation m_pColorGradingLUTTextureImageAllocation;
};

struct ST_StorageBuffer
{
	// limits
	uint32_t m_minUniformBufferOffsetAlignment{ 256 };
	uint32_t m_minStorageBufferOffsetAlignment{ 256 };
	uint32_t m_maxStorageBufferRange{ 1 << 27 };
	uint32_t m_nonCoherentAtomSize{ 256 };	// 非一致性原子大小

	RHIBuffer* m_pGlobalUploadRingbuffer;	// 全局上传环形缓冲区
	RHIDeviceMemory* m_pGlobalUploadRingbufferMemory;
	void* m_pGlobalUploadRingbufferMemoryPointer;
	std::vector<uint32_t> m_globalUploadRingbuffersBegin;
	std::vector<uint32_t> m_globalUploadRingbuffersEnd;
	std::vector<uint32_t> m_globalUploadRingbuffersSize;

	// 
	RHIBuffer* m_pGlobalNullDescriptorStorageBuffer;
	RHIDeviceMemory* m_pGlobalNullDescriptorStorageBufferMemory;

	// axis
	RHIBuffer* m_pAxisInefficientStorageBuffer;
	RHIDeviceMemory* m_pAxisInefficientStorageBufferMemory;
	void* m_pAxisInefficientStorageBufferMemoryPointer;
};

struct ST_GlobalRenderResource
{
	ST_IBLResource m_iblResource;	// IBL资源
	ST_ColorGradingResource m_colorGradingResource;
	ST_StorageBuffer m_storageBuffer;
};

class RenderResource : public RenderResourceBase
{
public:
	void Clear() override final;	// 清理渲染资源

	virtual void UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc) override final;	// 上传全局渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data) override final;	// 上传游戏对象渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data) override final;	// 上传游戏对象渲染资源（仅网格数据）

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data) override final; // 上传游戏对象渲染资源（仅材质数据）

	virtual void UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) override final; // 更新每帧缓冲区

	void ResetRingBufferOffset(uint8_t currentFrameIndex);	// 重置环形缓冲区偏移

private:

};

NAMESPACE_XYH_END