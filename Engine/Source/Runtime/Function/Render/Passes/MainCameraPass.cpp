#include "MainCameraPass.h"

NAMESPACE_XYH_BEGIN

void MainCameraPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);	// ???

	const ST_MainCameraPassInitInfp* mainCameraPassInitInfo = static_cast<const ST_MainCameraPassInitInfp*>(initInfo);
	m_enableFXAA = mainCameraPassInitInfo->m_enableFXAA;

	SetupAttachments();
	SetupRenderPass();
	SetupDescriptorSetLayout();
	SetupPipelines();
	SetupDescriptorSet();
	SetupFramebufferDescriptorSet();
	SetupSwapchainFramebuffers();

	SetupParticlePass();
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

void MainCameraPass::SetupParticlePass()
{

}

void MainCameraPass::SetupAttachments()
{
	m_framebuffer.m_attachments.resize(_main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count);	// 5 + 2 = 7

	m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;	// GBuffer A: 位置
	m_framebuffer.m_attachments[_main_camera_pass_gbuffer_b].m_format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;	// GBuffer B: 法线
	m_framebuffer.m_attachments[_main_camera_pass_gbuffer_c].m_format = ERHIFormat::RHI_FORMAT_R8G8B8A8_SRGB;	// GBuffer C: 颜色
	m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_odd].m_format = ERHIFormat::RHI_FORMAT_R16G16B16A16_SFLOAT;	// 备用缓冲区 奇数
	m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_even].m_format = ERHIFormat::RHI_FORMAT_R16G16B16A16_SFLOAT;	// 备用缓冲区 偶数
	
	for (int bufferIndex = 0; bufferIndex < _main_camera_pass_custom_attachment_count; ++bufferIndex)
	{
		if (bufferIndex == _main_camera_pass_gbuffer_a)
		{
			m_pRHI->CreateImage(
				m_pRHI->GetSwapchainInfo().m_extent.m_height,
				m_pRHI->GetSwapchainInfo().m_extent.m_width,	// 交换链的宽度和高度
				m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_format,	// 图像格式
				ERHIImageTiling::RHI_IMAGE_TILING_OPTIMAL,	// 最佳布局
				ERHIImageUsageFlagBits::RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | ERHIImageUsageFlagBits::RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | ERHIImageUsageFlagBits::RHI_IMAGE_USAGE_TRANSFER_SRC_BIT,	// 用作输入附件、颜色附件和传输源
				ERHIMemoryPropertyFlagBits::RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// 位于设备本地内存中
				m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_pImage,	// 图像
				m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_pMemory,	// 图像内存
				0,	// 图像创建标志
				1,	// 图像数组层数
				1);	// 图像miplevels
		}
		else
		{
			m_pRHI->CreateImage(
				m_pRHI->GetSwapchainInfo().m_extent.m_height, 
				m_pRHI->GetSwapchainInfo().m_extent.m_width,
				m_framebuffer.m_attachments[bufferIndex].m_format,
				ERHIImageTiling::RHI_IMAGE_TILING_OPTIMAL,	// 最佳布局
				ERHIImageUsageFlagBits::RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,	// 用作颜色附件、输入附件和临时附件
				ERHIMemoryPropertyFlagBits::RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// 位于设备本地内存中
				m_framebuffer.m_attachments[bufferIndex].m_pImage,	// 图像
				m_framebuffer.m_attachments[bufferIndex].m_pMemory,	// 图像内存
				0,	// 图像创建标志
				1,	// 图像数组层数
				1);	// 图像miplevels
		}

		m_pRHI->CreateImageView(
			m_framebuffer.m_attachments[bufferIndex].m_pImage,	// 图像
			m_framebuffer.m_attachments[bufferIndex].m_format,	// 图像格式
			ERHIImageAspectFlagBits::RHI_IMAGE_ASPECT_COLOR_BIT,	// 图像方面标志位 为颜色
			ERHIImageViewType::RHI_IMAGE_VIEW_TYPE_2D,	// 图像视图类型 为2D
			1,	// 图像视图布局数量
			1,	// 图像视图miplevels
			m_framebuffer.m_attachments[bufferIndex].m_pView);	// 图像视图
	}

	m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_odd].m_format = RHI_FORMAT_R16G16B16A16_SFLOAT;	// 后处理缓冲区 奇数
	m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_even].m_format = RHI_FORMAT_R16G16B16A16_SFLOAT;	// 后处理缓冲区 偶数
	// 创建后处理缓冲区图像和图像视图
	for (int attachmentIndex = _main_camera_pass_custom_attachment_count; attachmentIndex < _main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count; ++attachmentIndex)
	{

	}
}

void MainCameraPass::SetupRenderPass()
{
}

void MainCameraPass::SetupDescriptorSetLayout()
{
}

void MainCameraPass::SetupPipelines()
{
}

void MainCameraPass::SetupDescriptorSet()
{
}

void MainCameraPass::SetupFramebufferDescriptorSet()
{
}

void MainCameraPass::SetupSwapchainFramebuffers()
{
}

void MainCameraPass::SetupModelGlobalDescriptorSet()
{
}

void MainCameraPass::SetupSkyboxDescriptorSet()
{
}

void MainCameraPass::SetupAxisDescriptorSet()
{
}

void MainCameraPass::SetupParticleDescriptorSet()
{
}

void MainCameraPass::SetupGbufferLightingDescriptorSet()
{
}

void MainCameraPass::DrawMeshGbuffer()
{
}

void MainCameraPass::DrawDeferredLighting()
{
}

void MainCameraPass::DrawMeshLighting()
{
}

void MainCameraPass::DrawSkybox()
{
}

void MainCameraPass::DrawAxis()
{
}

NAMESPACE_XYH_END

