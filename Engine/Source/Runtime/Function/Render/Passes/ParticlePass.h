#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"
#include "Runtime/Function/Render/RenderResource.h"
#include "Runtime/Function/Particle/ParticleManager.h"

NAMESPACE_XYH_BEGIN

struct ST_ParticlePassInitInfo : public ST_RenderPassInitInfo
{
	std::shared_ptr<ParticleManager> m_pParticleManager;
};

// 色调映射渲染通道
class ParticlePass : public RenderPass
{
public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void SetRenderCommandBufferHandle(RHICommandBuffer* commandBuffer);

	void CopyNormalAndDepthImage();

	void SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage);

	void SetupParticlePass();

	void Simulate();

	void SetRenderPassHandle(RHIRenderPass* pRenderPass);

	void UpdateAfterFramebufferRecreate();

private:

	// 准备统一缓冲区
	// 用于传递在多次绘制调用中保持不变或变化缓慢的数据
	void PrepareUniformBuffer();

	void SetupDescriptorSetLayout();

	void SetupPipelines();

	void SetupAttachments();

private:
	std::shared_ptr<ParticleManager> m_pParticleManager;

	RHICommandBuffer* m_pComputeCommandBuffer = nullptr;
	RHICommandBuffer* m_pRenderCommandBuffer = nullptr;
	RHICommandBuffer* m_pCopyCommandBuffer = nullptr;

	RHIFence* m_pFence = nullptr;

	ST_ParticleBillboardPerframeStorageBufferObject m_particleBillboardPerframeStorageBufferObject;	// 
	ST_ParticleCollisionPerframeStorageBufferObject m_particleCollisionPerframeStorageBufferObject;	// 粒子控制数据

	// 内存属性
	RHIBuffer* m_pSceneUniformBuffer = nullptr;
	RHIBuffer* m_pComputeUniformBuffer = nullptr;
	RHIBuffer* m_pParticleBillboardUniformBuffer = nullptr;

	// 内存映射首地址指针
	void* m_pSceneUniformBufferMapped = nullptr;
	void* m_pParticleComputeBufferMapped = nullptr;
	void* m_pParticleBillboardUniformBufferMapped = nullptr;

	struct ST_uVec4
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t w;
	};
	struct ST_ComputeUniformBufferObject
	{
		int m_emitGap;	// 
		int m_xemitCount;
		float m_maxLife;
		float m_timeStep;
		Vector4 m_pack; // randomness 3 | frame index 1
		Vector3 m_gravity;
		float m_padding;
		ST_uVec4 m_viewport; // x, y, width, height
		Vector4 m_extent;   // width, height, near, far
	} m_ubo;


	ST_RHIViewport m_viewportParams;	// 交换链视口属性

	// 管线对象
	RHIPipeline* m_kickoffPipeline = nullptr;	// 开始
	RHIPipeline* m_emitPipeline = nullptr;	// 发出
	RHIPipeline* m_simulatePipeline = nullptr;	// 模拟

	RHIRenderPass* m_pRenderPass = nullptr;

	// 粒子绘制的image
	RHIImage* m_pParticleBillboardTextureImage = nullptr;
	RHIImageView* m_pParticleBillboardTextureImageView = nullptr;
	VmaAllocation m_particleBillboardTextureVmaAllocation;

	// Test
	RHIImage* m_pPiccoloLogoTextureImage = nullptr;
	RHIImageView* m_pPiccoloLogoTextureImageView = nullptr;
	VmaAllocation m_piccoloLogoTextureVmaAllocation;

	RHIImage* m_pSrcDepthImage = nullptr;
	RHIImage* m_pSrcNormalImage = nullptr;

	RHIImage* m_pDstNormalImage = nullptr;
	RHIImage* m_pDstDepthImage = nullptr;
	RHIDeviceMemory* m_pDstNormalImageMemory = nullptr;
	RHIDeviceMemory* m_pDstDepthImageMemory = nullptr;

	RHIImageView* m_pSrcDepthImageView = nullptr;
	RHIImageView* m_pSrcNormalImageView = nullptr;

};

NAMESPACE_XYH_END