#include "RenderPipelineBase.h"
#include "RenderResourceBase.h"
#include "Interface/RHI.h"

NAMESPACE_XYH_BEGIN

void RenderPipelineBase::PreparePassData(std::shared_ptr<RenderResourceBase> pRenderResource)
{
	m_pMainCameraPass->PreparePassData(pRenderResource);
	m_pPickPass->PreparePassData(pRenderResource);
	m_pDirectionalLightPass->PreparePassData(pRenderResource);
	m_pPointLightShadowPass->PreparePassData(pRenderResource);
	m_pParticlePass->PreparePassData(pRenderResource);

	// g_runtime_global_context.m_debugdraw_manager->preparePassData(render_resource);
}

void RenderPipelineBase::ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

void RenderPipelineBase::DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

NAMESPACE_XYH_END


