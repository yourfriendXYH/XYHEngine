#include "RenderPipeline.h"
#include "Passes/PointLightPass.h"
#include "Passes/DirectionalLightPass.h"
#include "Passes/MainCameraPass.h"
#include "Passes/ToneMappingPass.h"
#include "Passes/ColorGradingPass.h"
#include "Passes/UIPass.h"
#include "Passes/CombineUIPass.h"
#include "Passes/PickPass.h"
#include "Passes/FXAAPass.h"
#include "Passes/ParticlePass.h"

NAMESPACE_XYH_BEGIN

void RenderPipeline::Initialize(RenderPipelineInitInfo initInfo)
{
	m_pPointLightShadowPass = std::make_shared<PointLightShadowPass>();
	m_pDirectionalLightPass = std::make_shared<DirectionalLightShadowPass>();
	m_pMainCameraPass = std::make_shared<MainCameraPass>();
	m_pToneMappingPass = std::make_shared<ToneMappingPass>();
	m_pColorGradingPass = std::make_shared<ColorGradingPass>();
	m_pUIPass = std::make_shared<UIPass>();
	m_pCombineUIPass = std::make_shared<CombineUIPass>();
	m_pPickPass = std::make_shared<PickPass>();
	m_pFxaaPass = std::make_shared<FXAAPass>();
	m_pParticlePass = std::make_shared<ParticlePass>();
}

void RenderPipeline::ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

void RenderPipeline::DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

uint32_t RenderPipeline::GetGuidOfPickedMesh(const Vector2& pickedUV)
{
	return 0;
}

void RenderPipeline::PassUpdateAfterRecreateSwapchain()
{
}

void RenderPipeline::SetAxisVisibleState(bool state)
{
}

void RenderPipeline::SetSelectedAxis(size_t selectedAxis)
{
}

NAMESPACE_XYH_END


