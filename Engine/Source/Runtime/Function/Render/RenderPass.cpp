#include "RenderPass.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"

XYH::ST_VisibleNodes XYH::RenderPass::s_visibleNodes;

NAMESPACE_XYH_BEGIN

void RenderPass::Initialize(const ST_RenderPassInitInfo* init_info)
{
}

void RenderPass::PostInitialize()
{
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
		layouts.push_back(descriptor.m_descriptorSetLayout);
	}
	return layouts;
}

NAMESPACE_XYH_END

