#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"
#include "Runtime/Function/Render/RenderResource.h"
#include "Runtime/Function/Particle/ParticleManager.h"
#include <Runtime/Function/Particle/ParticleDesc.h>

NAMESPACE_XYH_BEGIN

struct ST_ParticlePassInitInfo : public ST_RenderPassInitInfo
{
	std::shared_ptr<ParticleManager> m_pParticleManager;
};

class ParticleEmitterBufferBatch
{
public:
	RHIBuffer* m_position_render_buffer = nullptr;
	RHIBuffer* m_position_device_buffer = nullptr;
	RHIBuffer* m_position_host_buffer = nullptr;
	RHIBuffer* m_counter_device_buffer = nullptr;
	RHIBuffer* m_counter_host_buffer = nullptr;
	RHIBuffer* m_indirect_dispatch_argument_buffer = nullptr;
	RHIBuffer* m_alive_list_buffer = nullptr;
	RHIBuffer* m_alive_list_next_buffer = nullptr;
	RHIBuffer* m_dead_list_buffer = nullptr;
	RHIBuffer* m_particle_component_res_buffer = nullptr;

	RHIDeviceMemory* m_counter_host_memory = nullptr;
	RHIDeviceMemory* m_position_host_memory = nullptr;
	RHIDeviceMemory* m_position_device_memory = nullptr;
	RHIDeviceMemory* m_counter_device_memory = nullptr;
	RHIDeviceMemory* m_indirect_dispatch_argument_memory = nullptr;
	RHIDeviceMemory* m_alive_list_memory = nullptr;
	RHIDeviceMemory* m_alive_list_next_memory = nullptr;
	RHIDeviceMemory* m_dead_list_memory = nullptr;
	RHIDeviceMemory* m_particle_component_res_memory = nullptr;
	RHIDeviceMemory* m_position_render_memory = nullptr;

	void* m_pEmitterDescMapped{ nullptr };

	ST_ParticleEmitterDesc m_emitterDesc;

	uint32_t m_num_particle{ 0 };
	void FreeUpBatch(std::shared_ptr<RHI> rhi) {};
};

// 色调映射渲染通道
class ParticlePass : public RenderPass
{
public:
	// 初始化（传入全局资源、粒子管理器）
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void PreparePassData(std::shared_ptr<RenderResourceBase> pRenderResource) override final;

	// 设置渲染命令缓冲区指针
	void SetRenderCommandBufferHandle(RHICommandBuffer* commandBuffer);

	void CopyNormalAndDepthImage();

	void SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage);

	// 粒子Pass部分数据创建
	void SetupParticlePass();

	void Simulate();

	void SetRenderPassHandle(RHIRenderPass* pRenderPass);

	void UpdateAfterFramebufferRecreate();

private:

	// 准备统一缓冲区
	// 用于传递在多次绘制调用中保持不变或变化缓慢的数据
	void PrepareUniformBuffer();

	// 创建描述符布局
	void SetupDescriptorSetLayout();

	// 创建图形管线、计算管线
	void SetupPipelines();

	// 创建附件
	void SetupAttachments();

	// 更新 m_ubo 数据
	void UpdateUniformBuffer();

	// 更新发射器的变换数据
	void UpdateEmitterTransform();

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
	RHIPipeline* m_pKickoffPipeline = nullptr;	// 开始
	RHIPipeline* m_pEmitPipeline = nullptr;	// 发出
	RHIPipeline* m_pSimulatePipeline = nullptr;	// 模拟

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

	std::vector<ST_ParticleEmitterTransformDesc> m_emitterTransformIndices;

	std::vector<ParticleEmitterBufferBatch> m_emitterBufferBatches;
};

NAMESPACE_XYH_END