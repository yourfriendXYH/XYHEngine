#include "MainCameraPass.h"

NAMESPACE_XYH_BEGIN

void MainCameraPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);	// ???

	const ST_MainCameraPassInitInfp* mainCameraPassInitInfo = static_cast<const ST_MainCameraPassInitInfp*>(initInfo);
	m_enableFXAA = mainCameraPassInitInfo->m_enableFXAA;

	SetupAttachments();	// 创建帧缓冲的图像及视图

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
				m_pRHI->GetSwapchainInfo().m_extent.m_width,
				m_pRHI->GetSwapchainInfo().m_extent.m_height,	// 交换链的宽度和高度
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
				m_pRHI->GetSwapchainInfo().m_extent.m_width,
				m_pRHI->GetSwapchainInfo().m_extent.m_height,
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
		m_pRHI->CreateImage(
			m_pRHI->GetSwapchainInfo().m_extent.m_width,
			m_pRHI->GetSwapchainInfo().m_extent.m_height,
			m_framebuffer.m_attachments[attachmentIndex].m_format,
			ERHIImageTiling::RHI_IMAGE_TILING_OPTIMAL,	// 最佳布局
			ERHIImageUsageFlagBits::RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | RHI_IMAGE_USAGE_SAMPLED_BIT,
			ERHIMemoryPropertyFlagBits::RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,	// 位于设备本地内存中
			m_framebuffer.m_attachments[attachmentIndex].m_pImage,
			m_framebuffer.m_attachments[attachmentIndex].m_pMemory,
			0,
			1,
			1);

		m_pRHI->CreateImageView(
			m_framebuffer.m_attachments[attachmentIndex].m_pImage,
			m_framebuffer.m_attachments[attachmentIndex].m_format,
			ERHIImageAspectFlagBits::RHI_IMAGE_ASPECT_COLOR_BIT,
			ERHIImageViewType::RHI_IMAGE_VIEW_TYPE_2D,
			1,
			1,
			m_framebuffer.m_attachments[attachmentIndex].m_pView);
	}
}

void MainCameraPass::SetupRenderPass()
{
	ST_RHIAttachmentDescription attachments[_main_camera_pass_attachment_count] = {};

	// 法线的GBuffer描述
	ST_RHIAttachmentDescription& gbufferNormalAttachmentDescription = attachments[_main_camera_pass_gbuffer_a];
	gbufferNormalAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_format;	// 图像格式
	gbufferNormalAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	gbufferNormalAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	gbufferNormalAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_STORE;	// 存储操作 存储
	gbufferNormalAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	gbufferNormalAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	gbufferNormalAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	gbufferNormalAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 金属度-粗糙度的GBuffer描述
	ST_RHIAttachmentDescription& gbufferMetallicRoughnessShadingmodeidAttachmentDescription = attachments[_main_camera_pass_gbuffer_b];
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_b].m_format;	// 图像格式
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	gbufferMetallicRoughnessShadingmodeidAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 颜色的GBuffer描述
	ST_RHIAttachmentDescription& gbufferAlbedoAttachmentDescription = attachments[_main_camera_pass_gbuffer_c];
	gbufferAlbedoAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_c].m_format;	// 图像格式
	gbufferAlbedoAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	gbufferAlbedoAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	gbufferAlbedoAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	gbufferAlbedoAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	gbufferAlbedoAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	gbufferAlbedoAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	gbufferAlbedoAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 备用缓冲区 偶数 描述
	ST_RHIAttachmentDescription& backupOddColorAttachmentDescription = attachments[_main_camera_pass_backup_buffer_odd];
	backupOddColorAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_odd].m_format;	// 图像格式
	backupOddColorAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	backupOddColorAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	backupOddColorAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	backupOddColorAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	backupOddColorAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	backupOddColorAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	backupOddColorAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 备用缓冲区 奇数 描述
	ST_RHIAttachmentDescription& backupEvenColorAttachmentDescription = attachments[_main_camera_pass_backup_buffer_even];
	backupEvenColorAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_even].m_format;	// 图像格式
	backupEvenColorAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	backupEvenColorAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	backupEvenColorAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	backupEvenColorAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	backupEvenColorAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	backupEvenColorAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	backupEvenColorAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 后处理缓冲区 偶数 描述
	ST_RHIAttachmentDescription& postProcessOddColorAttachmentDescription = attachments[_main_camera_pass_post_process_buffer_odd];
	postProcessOddColorAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_odd].m_format;	// 图像格式
	postProcessOddColorAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	postProcessOddColorAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	postProcessOddColorAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	postProcessOddColorAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	postProcessOddColorAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	postProcessOddColorAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	postProcessOddColorAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 后处理缓冲区 奇数 描述
	ST_RHIAttachmentDescription& postProcessEvenColorAttachmentDescription = attachments[_main_camera_pass_post_process_buffer_even];
	postProcessEvenColorAttachmentDescription.m_format = m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_odd].m_format;	// 图像格式
	postProcessEvenColorAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	postProcessEvenColorAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	postProcessEvenColorAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 存储操作 不关心
	postProcessEvenColorAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	postProcessEvenColorAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	postProcessEvenColorAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	postProcessEvenColorAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 最终布局 着色器只读最优

	// 深度附件描述
	ST_RHIAttachmentDescription& depthAttachmentDescription = attachments[_main_camera_pass_depth];
	depthAttachmentDescription.m_format = m_pRHI->GetDepthImageInfo().m_depthImageFormat;
	depthAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	depthAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	depthAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_STORE;	// 存储操作 存储
	depthAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	depthAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	depthAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	depthAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 最终布局 深度模板附件最优

	// 交换链图像描述
	ST_RHIAttachmentDescription& swapchainImageAttachmentDescription = attachments[_main_camera_pass_swap_chain_image];	// 交换链图像描述
	swapchainImageAttachmentDescription.m_format = m_pRHI->GetSwapchainInfo().m_imageFormat;	// 图像格式
	swapchainImageAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	swapchainImageAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	swapchainImageAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_STORE;	// 存储操作 存储
	swapchainImageAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	swapchainImageAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	swapchainImageAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	swapchainImageAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// 最终布局 呈现源

	// 子通道描述
	ST_RHISubpassDescription subpasses[_main_camera_subpass_count] = {};
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

