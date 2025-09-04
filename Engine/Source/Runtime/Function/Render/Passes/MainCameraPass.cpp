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

	// Main Camera Pass 的第一个子通道 基础通道
	ST_RHIAttachmentReference basePassColorAttachmentsReference[3] = {};	// 3个颜色附件引用
	basePassColorAttachmentsReference[0].m_attachment = &gbufferNormalAttachmentDescription - attachments;;	// 地址偏移值
	basePassColorAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优
	basePassColorAttachmentsReference[1].m_attachment = &gbufferMetallicRoughnessShadingmodeidAttachmentDescription - attachments;	// 地址偏移值
	basePassColorAttachmentsReference[1].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优
	basePassColorAttachmentsReference[2].m_attachment = &gbufferAlbedoAttachmentDescription - attachments;	// 地址偏移值
	basePassColorAttachmentsReference[2].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHIAttachmentReference basePassDepthAttachmentReference = {};	// 深度附件引用
	basePassDepthAttachmentReference.m_attachment = &depthAttachmentDescription - attachments;	// 地址偏移值
	basePassDepthAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 深度模板附件最优

	ST_RHISubpassDescription& basePass = subpasses[_main_camera_subpass_basepass];
	basePass.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;	// 图形管线绑定点
	basePass.m_colorAttachmentCount = sizeof(basePassColorAttachmentsReference) / sizeof(basePassColorAttachmentsReference[0]);	// 颜色附件数量
	basePass.m_pColorAttachments = &basePassColorAttachmentsReference[0];	// 颜色附件引用
	basePass.m_pDepthStencilAttachment = &basePassDepthAttachmentReference;	// 深度附件引用
	basePass.m_preserveAttachmentCount = 0;
	basePass.m_pPreserveAttachments = nullptr;

	// Main Camera Pass 的第二个子通道 延迟光照通道
	ST_RHIAttachmentReference deferredLightingPassInputAttachmentsReference[4] = {};
	deferredLightingPassInputAttachmentsReference[0].m_attachment = &gbufferNormalAttachmentDescription - attachments;
	deferredLightingPassInputAttachmentsReference[0].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[1].m_attachment = &gbufferMetallicRoughnessShadingmodeidAttachmentDescription - attachments;
	deferredLightingPassInputAttachmentsReference[1].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[2].m_attachment = &gbufferAlbedoAttachmentDescription - attachments;
	deferredLightingPassInputAttachmentsReference[2].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[3].m_attachment = &depthAttachmentDescription - attachments;
	deferredLightingPassInputAttachmentsReference[3].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优

	ST_RHIAttachmentReference deferredLightingPassColorAttachmentReference[1] = {};
	deferredLightingPassColorAttachmentReference[0].m_attachment = &backupOddColorAttachmentDescription - attachments;
	deferredLightingPassColorAttachmentReference[0].m_layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHISubpassDescription& deferredLightingPass = subpasses[_main_camera_subpass_deferred_lighting];
	deferredLightingPass.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;	// 图形管线绑定点
	deferredLightingPass.m_inputAttachmentCount = sizeof(deferredLightingPassInputAttachmentsReference) / sizeof(deferredLightingPassInputAttachmentsReference[0]);	// 输入附件数量
	deferredLightingPass.m_pInputAttachments = &deferredLightingPassInputAttachmentsReference[0];	// 输入附件引用
	deferredLightingPass.m_colorAttachmentCount = sizeof(deferredLightingPassColorAttachmentReference) / sizeof(deferredLightingPassColorAttachmentReference[0]);	// 颜色附件数量
	deferredLightingPass.m_pColorAttachments = &deferredLightingPassColorAttachmentReference[0];	// 颜色附件引用
	deferredLightingPass.m_pDepthStencilAttachment = nullptr;
	deferredLightingPass.m_preserveAttachmentCount = 0;	// 保留附件数量
	deferredLightingPass.m_pPreserveAttachments = nullptr;	// 保留附件引用

	// Main Camera Pass 的第三个子通道 前向光照通道
	ST_RHIAttachmentReference forwardLightingPassColorAttachmentsReference[1] = {};
	forwardLightingPassColorAttachmentsReference[0].m_attachment = &backupOddColorAttachmentDescription - attachments;
	forwardLightingPassColorAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHIAttachmentReference forwardLightingPassDepthAttachmentReference{};
	forwardLightingPassDepthAttachmentReference.m_attachment = &depthAttachmentDescription - attachments;
	forwardLightingPassDepthAttachmentReference.m_layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 深度模板附件最优

	ST_RHISubpassDescription& forwardLightingPass = subpasses[_main_camera_subpass_forward_lighting];
	forwardLightingPass.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;	// 图形管线绑定点
	forwardLightingPass.m_inputAttachmentCount = 0;	// 输入附件数量
	forwardLightingPass.m_pInputAttachments = nullptr;	// 输入附件引用
	forwardLightingPass.m_colorAttachmentCount = sizeof(forwardLightingPassColorAttachmentsReference) / sizeof(forwardLightingPassColorAttachmentsReference[0]);	// 颜色附件数量
	forwardLightingPass.m_pColorAttachments = &forwardLightingPassColorAttachmentsReference[0];	// 颜色附件引用
	forwardLightingPass.m_pDepthStencilAttachment = &forwardLightingPassDepthAttachmentReference;	// 深度附件引用
	forwardLightingPass.m_preserveAttachmentCount = 0;	// 保留附件数量
	forwardLightingPass.m_pPreserveAttachments = nullptr;	// 保留附件引用

	// 色调映射通道
	ST_RHIAttachmentReference toneMappingPassInputAttachmentReference{};
	toneMappingPassInputAttachmentReference.m_attachment = &backupOddColorAttachmentDescription - attachments;
	toneMappingPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference toneMappingPassColorAttachmentReference{};
	toneMappingPassColorAttachmentReference.m_attachment = &backupEvenColorAttachmentDescription - attachments;
	toneMappingPassColorAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHISubpassDescription& toneMappingPass = subpasses[_main_camera_subpass_tone_mapping];
	toneMappingPass.m_pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
	toneMappingPass.m_inputAttachmentCount = 1;
	toneMappingPass.m_pInputAttachments = &toneMappingPassInputAttachmentReference;
	toneMappingPass.m_colorAttachmentCount = 1;
	toneMappingPass.m_pColorAttachments = &toneMappingPassColorAttachmentReference;
	toneMappingPass.m_pDepthStencilAttachment = nullptr;
	toneMappingPass.m_preserveAttachmentCount = 0;
	toneMappingPass.m_pPreserveAttachments = nullptr;

	// 颜色分级通道
	ST_RHIAttachmentReference colorGradingPassInputAttachmentReference{};
	colorGradingPassInputAttachmentReference.m_attachment = &backupEvenColorAttachmentDescription - attachments;
	colorGradingPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference colorGradingPassColorAttachmentReference{};
	if (m_enableFXAA)
	{
		colorGradingPassColorAttachmentReference.m_attachment = &postProcessOddColorAttachmentDescription - attachments;	// 输出到后处理 偶数
	}
	else
	{
		colorGradingPassColorAttachmentReference.m_attachment = &backupOddColorAttachmentDescription - attachments;
	}
	colorGradingPassColorAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	ST_RHISubpassDescription& colorGradingPass = subpasses[_main_camera_subpass_color_grading];
	colorGradingPass.m_pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
	colorGradingPass.m_inputAttachmentCount = 1;
	colorGradingPass.m_pInputAttachments = &colorGradingPassInputAttachmentReference;
	colorGradingPass.m_colorAttachmentCount = 1;
	colorGradingPass.m_pColorAttachments = &colorGradingPassColorAttachmentReference;
	colorGradingPass.m_pDepthStencilAttachment = nullptr;
	colorGradingPass.m_preserveAttachmentCount = 0;
	colorGradingPass.m_pPreserveAttachments = nullptr;

	// FXAA通道 
	ST_RHIAttachmentReference fxaaPassInputAttachmentReference{};
	if (m_enableFXAA)
	{
		fxaaPassInputAttachmentReference.m_attachment = &postProcessOddColorAttachmentDescription - attachments;	// 后处理 偶数
	}
	else
	{
		fxaaPassInputAttachmentReference.m_attachment = &backupEvenColorAttachmentDescription - attachments;	// 奇数
	}
	fxaaPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference fxaaPassColorAttachmentReference{};
	fxaaPassColorAttachmentReference.m_attachment = &backupOddColorAttachmentDescription - attachments;
	fxaaPassColorAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	ST_RHISubpassDescription& fxaaPass = subpasses[_main_camera_subpass_fxaa];
	fxaaPass.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;
	fxaaPass.m_inputAttachmentCount = 1;
	fxaaPass.m_pInputAttachments = &fxaaPassInputAttachmentReference;
	fxaaPass.m_colorAttachmentCount = 1;
	fxaaPass.m_pColorAttachments = &fxaaPassColorAttachmentReference;
	fxaaPass.m_pDepthStencilAttachment = nullptr;
	fxaaPass.m_preserveAttachmentCount = 0;
	fxaaPass.m_pPreserveAttachments = nullptr;

	// UI通道
	ST_RHIAttachmentReference uiPassColorAttachmentReference{};
	uiPassColorAttachmentReference.m_attachment = &backupEvenColorAttachmentDescription - attachments;
	uiPassColorAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	uint32_t uiPassPreserveAttachment = &backupOddColorAttachmentDescription - attachments;

	ST_RHISubpassDescription& uiPass = subpasses[_main_camera_subpass_ui];
	uiPass.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;
	uiPass.m_inputAttachmentCount = 0;
	uiPass.m_pInputAttachments = nullptr;
	uiPass.m_colorAttachmentCount = 1;
	uiPass.m_pColorAttachments = &uiPassColorAttachmentReference;
	uiPass.m_pDepthStencilAttachment = nullptr;
	uiPass.m_preserveAttachmentCount = 1;
	uiPass.m_pPreserveAttachments = &uiPassPreserveAttachment;

	// 
	ST_RHIAttachmentReference combineUIPassInputAttachmentsReference[2] = {};
	combineUIPassInputAttachmentsReference[0].m_attachment = &backupOddColorAttachmentDescription - attachments;
	combineUIPassInputAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	combineUIPassInputAttachmentsReference[1].m_attachment = &backupEvenColorAttachmentDescription - attachments;
	combineUIPassInputAttachmentsReference[1].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference combineUIPassColorAttachmentReference{};
	combineUIPassColorAttachmentReference.m_attachment = &swapchainImageAttachmentDescription - attachments;
	combineUIPassColorAttachmentReference.m_layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	ST_RHISubpassDescription& combineUIPass = subpasses[_main_camera_subpass_combine_ui];
	combineUIPass.m_pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
	combineUIPass.m_inputAttachmentCount = sizeof(combineUIPassInputAttachmentsReference) / sizeof(combineUIPassInputAttachmentsReference[0]);
	combineUIPass.m_pInputAttachments = combineUIPassInputAttachmentsReference;
	combineUIPass.m_colorAttachmentCount = 1;
	combineUIPass.m_pColorAttachments = &combineUIPassColorAttachmentReference;
	combineUIPass.m_pDepthStencilAttachment = nullptr;
	combineUIPass.m_preserveAttachmentCount = 0;
	combineUIPass.m_pPreserveAttachments = nullptr;

	// 依赖？？？
	ST_RHISubpassDependency dependencies[8] = {};

	ST_RHISubpassDependency& deferredLightingPassDependOnShadowMapPass = dependencies[0];
	deferredLightingPassDependOnShadowMapPass.m_srcSubpass = RHI_SUBPASS_EXTERNAL;
	deferredLightingPassDependOnShadowMapPass.m_dstSubpass = _main_camera_subpass_deferred_lighting;
	deferredLightingPassDependOnShadowMapPass.m_srcStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deferredLightingPassDependOnShadowMapPass.m_dstStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	deferredLightingPassDependOnShadowMapPass.m_srcAccessMask = ERHIAccessFlagBits::RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deferredLightingPassDependOnShadowMapPass.m_dstAccessMask = ERHIAccessFlagBits::RHI_ACCESS_SHADER_READ_BIT;
	deferredLightingPassDependOnShadowMapPass.m_dependencyFlags = 0; // NOT BY REGION
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

