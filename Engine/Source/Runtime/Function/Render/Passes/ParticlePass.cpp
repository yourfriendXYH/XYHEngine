#include "ParticlePass.h"

NAMESPACE_XYH_BEGIN

void ParticlePass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(initInfo);


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

