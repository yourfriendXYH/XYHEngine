#include "MainCameraPass.h"

NAMESPACE_XYH_BEGIN

void MainCameraPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
}

void MainCameraPass::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
{
}

void MainCameraPass::DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex)
{
}

void MainCameraPass::Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex)
{
	ST_RHIRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.m_sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
}

void MainCameraPass::SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass)
{
	m_pParticlePass = pParticlePass;
}

RHICommandBuffer* MainCameraPass::GetRenderCommandBuffer()
{
	return nullptr;
}

void MainCameraPass::UpdateAfterFramebufferRecreate()
{
}

NAMESPACE_XYH_END

