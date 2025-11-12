#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"
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

private:
	std::shared_ptr<ParticleManager> m_pParticleManager;

};

NAMESPACE_XYH_END