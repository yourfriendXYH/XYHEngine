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

	ST_ParticleCollisionPerframeStorageBufferObject m_particleCollisionPerframeStorageBufferObject;

	// 给描述符集用的数据
	RHIBuffer* m_pSceneUniformBuffer = nullptr;
	RHIBuffer* m_pComputeUniformBuffer = nullptr;

	void* m_pSceneUniformBufferMapped = nullptr;
	void* m_pParticleComputeBufferMapped = nullptr;

	struct ST_uVec4
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t w;
	};
	struct ST_ComputeUniformBufferObject
	{
		int     emit_gap;
		int     xemit_count;
		float   max_life;
		float   time_step;
		Vector4 pack; // randomness 3 | frame index 1
		Vector3 gravity;
		float m_padding;
		ST_uVec4 m_viewport; // x, y, width, height
		Vector4 extent;   // width, height, near, far
	} m_ubo;

};

NAMESPACE_XYH_END