#include "ParticlePass.h"

NAMESPACE_XYH_BEGIN

void ParticlePass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);

	const ST_ParticlePassInitInfo* pParticlePassInitInfo = static_cast<const ST_ParticlePassInitInfo*>(initInfo);
	assert(pParticlePassInitInfo != nullptr);
	m_pParticleManager = pParticlePassInitInfo->m_pParticleManager;	// 获取粒子管理器
}

void ParticlePass::SetRenderCommandBufferHandle(RHICommandBuffer* commandBuffer)
{
}

void ParticlePass::CopyNormalAndDepthImage()
{
}

void ParticlePass::SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage)
{
}

void ParticlePass::SetupParticlePass()
{
}

void ParticlePass::Simulate()
{
}

void ParticlePass::SetRenderPassHandle(RHIRenderPass* pRenderPass)
{
}

void ParticlePass::UpdateAfterFramebufferRecreate()
{
}

NAMESPACE_XYH_END

