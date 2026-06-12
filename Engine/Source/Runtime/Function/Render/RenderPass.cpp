#include "RenderPass.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"

XYH::ST_VisibleNodes XYH::RenderPass::s_visibleNodes;

NAMESPACE_XYH_BEGIN

RenderPass::RenderPass(ERenderPassType renderPassType, const char* name)
	:
	m_renderPassType(renderPassType),
	m_name(name)
{
}

void RenderPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	m_pGlobalRenderResource = &(std::static_pointer_cast<RenderResource>(m_pRenderResource)->m_globalRenderResource);
}

void RenderPass::PostInitialize()
{
}

void RenderPass::Build(uint32_t width, uint32_t height)
{
	switch (m_renderPassType)
	{
	case XYH::ERenderPassType::ERPT_GRAPHICS:
	{
		m_viewportWidth = width;
		m_viewportHeight = height;
	}
	break;
	case XYH::ERenderPassType::ERPT_COMPUTE:
	{

	}
	break;
	default:
		break;
	}
}

void RenderPass::Draw()
{
}

RHIRenderPass* RenderPass::GetRenderPass() const
{
	return m_framebuffer.m_pRenderPass;
}

std::vector<RHIImageView*> RenderPass::GetFramebufferImageViews() const
{
	std::vector<RHIImageView*> imageViews;
	for (const auto& attch : m_framebuffer.m_attachments)
	{
		imageViews.push_back(attch.m_pView);
	}
	return imageViews;
}

std::vector<RHIDescriptorSetLayout*> RenderPass::GetDescriptorSetLayouts() const
{
	std::vector<RHIDescriptorSetLayout*> layouts;
	for (const auto& descriptor : m_descriptorInfos)
	{
		layouts.push_back(descriptor.m_pDescriptorSetLayout);
	}
	return layouts;
}

NAMESPACE_XYH_END

