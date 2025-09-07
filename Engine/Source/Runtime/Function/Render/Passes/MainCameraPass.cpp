#include "MainCameraPass.h"

#include <MeshVert.h>
#include <MeshGBufferFrag.h>
#include "Runtime/Function/Render/RenderMesh.h"

NAMESPACE_XYH_BEGIN

void MainCameraPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);	// ???

	const ST_MainCameraPassInitInfp* mainCameraPassInitInfo = static_cast<const ST_MainCameraPassInitInfp*>(initInfo);
	m_enableFXAA = mainCameraPassInitInfo->m_enableFXAA;

	SetupAttachments();	// 创建帧缓冲的图像及视图

	SetupRenderPass();	// 创建渲染通道

	SetupDescriptorSetLayout();	// 创建描述符集布局(色器的缓存数据)

	SetupPipelines();	// 创建渲染管线

	SetupDescriptorSet();	// 设置描述符集

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
	basePassColorAttachmentsReference[0].m_attachment = static_cast<uint32_t>(&gbufferNormalAttachmentDescription - attachments);	// 地址偏移值
	basePassColorAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优
	basePassColorAttachmentsReference[1].m_attachment = static_cast<uint32_t>(&gbufferMetallicRoughnessShadingmodeidAttachmentDescription - attachments);	// 地址偏移值
	basePassColorAttachmentsReference[1].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优
	basePassColorAttachmentsReference[2].m_attachment = static_cast<uint32_t>(&gbufferAlbedoAttachmentDescription - attachments);	// 地址偏移值
	basePassColorAttachmentsReference[2].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHIAttachmentReference basePassDepthAttachmentReference = {};	// 深度附件引用
	basePassDepthAttachmentReference.m_attachment = static_cast<uint32_t>(&depthAttachmentDescription - attachments);	// 地址偏移值
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
	deferredLightingPassInputAttachmentsReference[0].m_attachment = static_cast<uint32_t>(&gbufferNormalAttachmentDescription - attachments);
	deferredLightingPassInputAttachmentsReference[0].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[1].m_attachment = static_cast<uint32_t>(&gbufferMetallicRoughnessShadingmodeidAttachmentDescription - attachments);
	deferredLightingPassInputAttachmentsReference[1].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[2].m_attachment = static_cast<uint32_t>(&gbufferAlbedoAttachmentDescription - attachments);
	deferredLightingPassInputAttachmentsReference[2].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优
	deferredLightingPassInputAttachmentsReference[3].m_attachment = static_cast<uint32_t>(&depthAttachmentDescription - attachments);
	deferredLightingPassInputAttachmentsReference[3].m_layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读最优

	ST_RHIAttachmentReference deferredLightingPassColorAttachmentReference[1] = {};
	deferredLightingPassColorAttachmentReference[0].m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
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
	forwardLightingPassColorAttachmentsReference[0].m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
	forwardLightingPassColorAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	ST_RHIAttachmentReference forwardLightingPassDepthAttachmentReference{};
	forwardLightingPassDepthAttachmentReference.m_attachment = static_cast<uint32_t>(&depthAttachmentDescription - attachments);
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
	toneMappingPassInputAttachmentReference.m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
	toneMappingPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference toneMappingPassColorAttachmentReference{};
	toneMappingPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&backupEvenColorAttachmentDescription - attachments);
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
	colorGradingPassInputAttachmentReference.m_attachment = static_cast<uint32_t>(&backupEvenColorAttachmentDescription - attachments);
	colorGradingPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference colorGradingPassColorAttachmentReference{};
	if (m_enableFXAA)
	{
		colorGradingPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&postProcessOddColorAttachmentDescription - attachments);	// 输出到后处理 偶数
	}
	else
	{
		colorGradingPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
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
		fxaaPassInputAttachmentReference.m_attachment = static_cast<uint32_t>(&postProcessOddColorAttachmentDescription - attachments);	// 后处理 偶数
	}
	else
	{
		fxaaPassInputAttachmentReference.m_attachment = static_cast<uint32_t>(&backupEvenColorAttachmentDescription - attachments);	// 奇数
	}
	fxaaPassInputAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference fxaaPassColorAttachmentReference{};
	fxaaPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
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
	uiPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&backupEvenColorAttachmentDescription - attachments);
	uiPassColorAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	uint32_t uiPassPreserveAttachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);

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
	combineUIPassInputAttachmentsReference[0].m_attachment = static_cast<uint32_t>(&backupOddColorAttachmentDescription - attachments);
	combineUIPassInputAttachmentsReference[0].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	combineUIPassInputAttachmentsReference[1].m_attachment = static_cast<uint32_t>(&backupEvenColorAttachmentDescription - attachments);
	combineUIPassInputAttachmentsReference[1].m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIAttachmentReference combineUIPassColorAttachmentReference{};
	combineUIPassColorAttachmentReference.m_attachment = static_cast<uint32_t>(&swapchainImageAttachmentDescription - attachments);
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

	ST_RHISubpassDependency& deferredLightingPassDependOnBasePass = dependencies[1];
	deferredLightingPassDependOnBasePass.m_srcSubpass = _main_camera_subpass_basepass;
	deferredLightingPassDependOnBasePass.m_dstSubpass = _main_camera_subpass_deferred_lighting;
	deferredLightingPassDependOnBasePass.m_srcStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deferredLightingPassDependOnBasePass.m_dstStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deferredLightingPassDependOnBasePass.m_srcAccessMask = ERHIAccessFlagBits::RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deferredLightingPassDependOnBasePass.m_dstAccessMask = ERHIAccessFlagBits::RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	deferredLightingPassDependOnBasePass.m_dependencyFlags = ERHIDependencyFlagBits::RHI_DEPENDENCY_BY_REGION_BIT;

	ST_RHISubpassDependency& forwardLightingPassDependOnDeferredLightingPass = dependencies[2];
	forwardLightingPassDependOnDeferredLightingPass.m_srcSubpass = _main_camera_subpass_deferred_lighting;
	forwardLightingPassDependOnDeferredLightingPass.m_dstSubpass = _main_camera_subpass_forward_lighting;
	forwardLightingPassDependOnDeferredLightingPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	forwardLightingPassDependOnDeferredLightingPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	forwardLightingPassDependOnDeferredLightingPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	forwardLightingPassDependOnDeferredLightingPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	forwardLightingPassDependOnDeferredLightingPass.m_dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

	ST_RHISubpassDependency& toneMappingPassDependOnLightingPass = dependencies[3];
	toneMappingPassDependOnLightingPass.m_srcSubpass = _main_camera_subpass_forward_lighting;
	toneMappingPassDependOnLightingPass.m_dstSubpass = _main_camera_subpass_tone_mapping;
	toneMappingPassDependOnLightingPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toneMappingPassDependOnLightingPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toneMappingPassDependOnLightingPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toneMappingPassDependOnLightingPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	toneMappingPassDependOnLightingPass.m_dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

	ST_RHISubpassDependency& colorGradingPassDependOnToneMappingPass = dependencies[4];
	colorGradingPassDependOnToneMappingPass.m_srcSubpass = _main_camera_subpass_tone_mapping;
	colorGradingPassDependOnToneMappingPass.m_dstSubpass = _main_camera_subpass_color_grading;
	colorGradingPassDependOnToneMappingPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorGradingPassDependOnToneMappingPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorGradingPassDependOnToneMappingPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	colorGradingPassDependOnToneMappingPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	colorGradingPassDependOnToneMappingPass.m_dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

	ST_RHISubpassDependency& fxaaPassDependOnColorGradingPass = dependencies[5];
	fxaaPassDependOnColorGradingPass.m_srcSubpass = _main_camera_subpass_color_grading;
	fxaaPassDependOnColorGradingPass.m_dstSubpass = _main_camera_subpass_fxaa;
	fxaaPassDependOnColorGradingPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fxaaPassDependOnColorGradingPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fxaaPassDependOnColorGradingPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	fxaaPassDependOnColorGradingPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	ST_RHISubpassDependency& uiPassDependOnFXAAPass = dependencies[6];
	uiPassDependOnFXAAPass.m_srcSubpass = _main_camera_subpass_fxaa;
	uiPassDependOnFXAAPass.m_dstSubpass = _main_camera_subpass_ui;
	uiPassDependOnFXAAPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	uiPassDependOnFXAAPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	uiPassDependOnFXAAPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	uiPassDependOnFXAAPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	uiPassDependOnFXAAPass.m_dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

	ST_RHISubpassDependency& combineUIPassDependOnUIPass = dependencies[7];
	combineUIPassDependOnUIPass.m_srcSubpass = _main_camera_subpass_ui;
	combineUIPassDependOnUIPass.m_dstSubpass = _main_camera_subpass_combine_ui;
	combineUIPassDependOnUIPass.m_srcStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	combineUIPassDependOnUIPass.m_dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	combineUIPassDependOnUIPass.m_srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	combineUIPassDependOnUIPass.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	combineUIPassDependOnUIPass.m_dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;


	ST_RHIRenderPassCreateInfo renderpassCreateInfo{};
	renderpassCreateInfo.m_sType = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.m_attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
	renderpassCreateInfo.m_pAttachments = attachments;
	renderpassCreateInfo.m_subpassCount = (sizeof(subpasses) / sizeof(subpasses[0]));
	renderpassCreateInfo.m_pSubpasses = subpasses;
	renderpassCreateInfo.m_dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
	renderpassCreateInfo.m_pDependencies = dependencies;
	if (m_pRHI->CreateRenderPass(&renderpassCreateInfo, m_framebuffer.m_pRenderPass) != RHI_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}
}

void MainCameraPass::SetupDescriptorSetLayout()
{
	m_descriptorInfos.resize(_layout_type_count);

	// 网格描述符集
	{
		ST_RHIDescriptorSetLayoutBinding meshMeshLayoutBindings[1];

		ST_RHIDescriptorSetLayoutBinding& meshMeshLayoutUniformBufferBinding = meshMeshLayoutBindings[0];	// 统一缓冲区绑定 赋值
		meshMeshLayoutUniformBufferBinding.m_binding = 0;
		meshMeshLayoutUniformBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;	// 存储缓冲区
		meshMeshLayoutUniformBufferBinding.m_descriptorCount = 1;
		meshMeshLayoutUniformBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;	// 顶点着色器
		meshMeshLayoutUniformBufferBinding.m_pImmutableSamplers = nullptr;	// 不可变采样器

		ST_RHIDescriptorSetLayoutCreateInfo meshMeshLayoutCreateInfo{};
		meshMeshLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		meshMeshLayoutCreateInfo.m_bindingCount = 1;
		meshMeshLayoutCreateInfo.m_pBindings = meshMeshLayoutBindings;

		// 创建 网格描述符集 布局
		if (m_pRHI->CreateDescriptorSetLayout(&meshMeshLayoutCreateInfo, m_descriptorInfos[_per_mesh].m_pDescriptorSetLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh mesh layout");
		}
	}

	// 全局描述符集
	{
		ST_RHIDescriptorSetLayoutBinding meshGlobalLayoutBindings[8];

		// 每帧存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerframeStorageBufferBinding = meshGlobalLayoutBindings[0];
		meshGlobalLayoutPerframeStorageBufferBinding.m_binding = 0;
		meshGlobalLayoutPerframeStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		meshGlobalLayoutPerframeStorageBufferBinding.m_descriptorCount = 1;
		meshGlobalLayoutPerframeStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
		meshGlobalLayoutPerframeStorageBufferBinding.m_pImmutableSamplers = nullptr;

		// 每次绘制存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerDrawcallStorageBufferBinding = meshGlobalLayoutBindings[1];
		meshGlobalLayoutPerDrawcallStorageBufferBinding.m_binding = 1;
		meshGlobalLayoutPerDrawcallStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;	// 动态存储缓冲区，允许在运行时更改缓冲区
		meshGlobalLayoutPerDrawcallStorageBufferBinding.m_descriptorCount = 1;
		meshGlobalLayoutPerDrawcallStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		meshGlobalLayoutPerDrawcallStorageBufferBinding.m_pImmutableSamplers = nullptr;

		// 顶点混合存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding = meshGlobalLayoutBindings[2];
		meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.m_binding = 2;
		meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.m_descriptorCount = 1;
		meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.m_pImmutableSamplers = nullptr;

		// BRDF LUT 纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutBrdfLUTTextureBinding = meshGlobalLayoutBindings[3];
		meshGlobalLayoutBrdfLUTTextureBinding.m_binding = 3;
		meshGlobalLayoutBrdfLUTTextureBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// 组合图像采样器，通常用于纹理采样
		meshGlobalLayoutBrdfLUTTextureBinding.m_descriptorCount = 1;
		meshGlobalLayoutBrdfLUTTextureBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		meshGlobalLayoutBrdfLUTTextureBinding.m_pImmutableSamplers = nullptr;

		// 环境光照纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutIrradianceTextureBinding = meshGlobalLayoutBindings[4];
		meshGlobalLayoutIrradianceTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
		meshGlobalLayoutIrradianceTextureBinding.m_binding = 4;

		// 镜面反射纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutSpecularTextureBinding = meshGlobalLayoutBindings[5];
		meshGlobalLayoutSpecularTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
		meshGlobalLayoutSpecularTextureBinding.m_binding = 5;

		// 点光源阴影纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutPointLightShadowTextureBinding = meshGlobalLayoutBindings[6];
		meshGlobalLayoutPointLightShadowTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
		meshGlobalLayoutPointLightShadowTextureBinding.m_binding = 6;

		// 直射光阴影纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutDirectionalLightShadowTextureBinding = meshGlobalLayoutBindings[7];
		meshGlobalLayoutDirectionalLightShadowTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
		meshGlobalLayoutDirectionalLightShadowTextureBinding.m_binding = 7;

		ST_RHIDescriptorSetLayoutCreateInfo meshGlobalLayoutCreateInfo;
		meshGlobalLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		meshGlobalLayoutCreateInfo.m_pNext = NULL;
		meshGlobalLayoutCreateInfo.m_flags = 0;
		meshGlobalLayoutCreateInfo.m_bindingCount = (sizeof(meshGlobalLayoutBindings) / sizeof(meshGlobalLayoutBindings[0]));
		meshGlobalLayoutCreateInfo.m_pBindings = meshGlobalLayoutBindings;

		// 创建 全局描述符集 布局
		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&meshGlobalLayoutCreateInfo, m_descriptorInfos[_mesh_global].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create mesh global layout");
		}
	}

	// 网格材质描述符集
	{
		ST_RHIDescriptorSetLayoutBinding meshMaterialLayoutBindings[6];

		// (set = 2, binding = 0 in fragment shader)
		// 材质统一缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutUniformBufferBinding = meshMaterialLayoutBindings[0];
		meshMaterialLayoutUniformBufferBinding.m_binding = 0;
		meshMaterialLayoutUniformBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		meshMaterialLayoutUniformBufferBinding.m_descriptorCount = 1;
		meshMaterialLayoutUniformBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		meshMaterialLayoutUniformBufferBinding.m_pImmutableSamplers = nullptr;

		// (set = 2, binding = 1 in fragment shader)
		// 基础颜色纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutBaseColorTextureBinding = meshMaterialLayoutBindings[1];
		meshMaterialLayoutBaseColorTextureBinding.m_binding = 1;
		meshMaterialLayoutBaseColorTextureBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		meshMaterialLayoutBaseColorTextureBinding.m_descriptorCount = 1;
		meshMaterialLayoutBaseColorTextureBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		meshMaterialLayoutBaseColorTextureBinding.m_pImmutableSamplers = nullptr;

		// (set = 2, binding = 2 in fragment shader)
		// 金属度-粗糙度纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutMetallicRoughnessTextureBinding = meshMaterialLayoutBindings[2];
		meshMaterialLayoutMetallicRoughnessTextureBinding = meshMaterialLayoutBaseColorTextureBinding;
		meshMaterialLayoutMetallicRoughnessTextureBinding.m_binding = 2;

		// (set = 2, binding = 3 in fragment shader)
		// 法线-粗糙度纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutNormalRoughnessTextureBinding = meshMaterialLayoutBindings[3];
		meshMaterialLayoutNormalRoughnessTextureBinding = meshMaterialLayoutBaseColorTextureBinding;
		meshMaterialLayoutNormalRoughnessTextureBinding.m_binding = 3;

		// (set = 2, binding = 4 in fragment shader)
		// 遮挡纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutOcclusionTexture_binding = meshMaterialLayoutBindings[4];
		meshMaterialLayoutOcclusionTexture_binding = meshMaterialLayoutBaseColorTextureBinding;
		meshMaterialLayoutOcclusionTexture_binding.m_binding = 4;

		// (set = 2, binding = 5 in fragment shader)
		// 自发光纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& meshMaterialLayoutEmissiveTextureBinding = meshMaterialLayoutBindings[5];
		meshMaterialLayoutEmissiveTextureBinding = meshMaterialLayoutBaseColorTextureBinding;
		meshMaterialLayoutEmissiveTextureBinding.m_binding = 5;

		// 创建 网格材质描述符集 布局
		ST_RHIDescriptorSetLayoutCreateInfo meshMaterialLayoutCreateInfo;
		meshMaterialLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		meshMaterialLayoutCreateInfo.m_pNext = nullptr;
		meshMaterialLayoutCreateInfo.m_flags = 0;
		meshMaterialLayoutCreateInfo.m_bindingCount = 6;
		meshMaterialLayoutCreateInfo.m_pBindings = meshMaterialLayoutBindings;

		if (m_pRHI->CreateDescriptorSetLayout(&meshMaterialLayoutCreateInfo, m_descriptorInfos[_mesh_per_material].m_pDescriptorSetLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh material layout");
		}
	}

	// 天空盒描述符集
	{
		ST_RHIDescriptorSetLayoutBinding skyboxLayoutBindings[2];

		// 每帧存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& skyboxLayoutPerframeStorageBufferBinding = skyboxLayoutBindings[0];
		skyboxLayoutPerframeStorageBufferBinding.m_binding = 0;
		skyboxLayoutPerframeStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		skyboxLayoutPerframeStorageBufferBinding.m_descriptorCount = 1;
		skyboxLayoutPerframeStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		skyboxLayoutPerframeStorageBufferBinding.m_pImmutableSamplers = nullptr;

		// 环境纹理绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& skyboxLayoutSpecularTextureBinding = skyboxLayoutBindings[1];
		skyboxLayoutSpecularTextureBinding.m_binding = 1;
		skyboxLayoutSpecularTextureBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		skyboxLayoutSpecularTextureBinding.m_descriptorCount = 1;
		skyboxLayoutSpecularTextureBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		skyboxLayoutSpecularTextureBinding.m_pImmutableSamplers = nullptr;

		// 创建 天空盒描述符集 布局
		ST_RHIDescriptorSetLayoutCreateInfo skyboxLayoutCreateInfo{};
		skyboxLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		skyboxLayoutCreateInfo.m_bindingCount = 2;
		skyboxLayoutCreateInfo.m_pBindings = skyboxLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&skyboxLayoutCreateInfo, m_descriptorInfos[_skybox].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create skybox layout");
		}
	}

	// 坐标轴描述符集
	{
		ST_RHIDescriptorSetLayoutBinding axisLayoutBindings[2];

		// 每帧存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& axisLayoutPerframeStorageBufferBinding = axisLayoutBindings[0];
		axisLayoutPerframeStorageBufferBinding.m_binding = 0;
		axisLayoutPerframeStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;	// 动态存储缓冲区，允许在运行时更改缓冲区
		axisLayoutPerframeStorageBufferBinding.m_descriptorCount = 1;
		axisLayoutPerframeStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		axisLayoutPerframeStorageBufferBinding.m_pImmutableSamplers = NULL;

		// 模型存储缓冲区绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& axisLayoutStorageBufferBinding = axisLayoutBindings[1];
		axisLayoutStorageBufferBinding.m_binding = 1;
		axisLayoutStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;	// 存储缓冲区，通常用于存储大量数据
		axisLayoutStorageBufferBinding.m_descriptorCount = 1;
		axisLayoutStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		axisLayoutStorageBufferBinding.m_pImmutableSamplers = NULL;

		// 创建 坐标轴描述符集 布局
		ST_RHIDescriptorSetLayoutCreateInfo axisLayoutCreateInfo{};
		axisLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		axisLayoutCreateInfo.m_bindingCount = 2;
		axisLayoutCreateInfo.m_pBindings = axisLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&axisLayoutCreateInfo, m_descriptorInfos[_axis].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create axis layout");
		}
	}

	// 延迟光照全局描述符集
	{
		ST_RHIDescriptorSetLayoutBinding gbufferLightingGlobalLayoutBindings[4];

		// 法线 输入附件 绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& gbufferNormalGlobalLayoutInputAttachmentBinding = gbufferLightingGlobalLayoutBindings[0];
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_binding = 0;
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;	// 输入附件，通常用于渲染管线中的子通道
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;

		// 金属度-粗糙度-遮挡 输入附件 绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding = gbufferLightingGlobalLayoutBindings[1];
		gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.m_binding = 1;
		gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;

		// 基础颜色 输入附件 绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& gbufferAlbedoGlobalLayoutInputAttachmentBinding = gbufferLightingGlobalLayoutBindings[2];
		gbufferAlbedoGlobalLayoutInputAttachmentBinding.m_binding = 2;
		gbufferAlbedoGlobalLayoutInputAttachmentBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		gbufferAlbedoGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferAlbedoGlobalLayoutInputAttachmentBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;

		// 深度 输入附件 绑定 赋值
		ST_RHIDescriptorSetLayoutBinding& gbufferDepthGlobalLayoutInputAttachmentBinding = gbufferLightingGlobalLayoutBindings[3];
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_binding = 3;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;

		// 创建 延迟光照全局描述符集 布局
		ST_RHIDescriptorSetLayoutCreateInfo gbufferLightingGlobalLayoutCreateInfo;
		gbufferLightingGlobalLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		gbufferLightingGlobalLayoutCreateInfo.m_pNext = NULL;
		gbufferLightingGlobalLayoutCreateInfo.m_flags = 0;
		gbufferLightingGlobalLayoutCreateInfo.m_bindingCount = sizeof(gbufferLightingGlobalLayoutBindings) / sizeof(gbufferLightingGlobalLayoutBindings[0]);
		gbufferLightingGlobalLayoutCreateInfo.m_pBindings = gbufferLightingGlobalLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&gbufferLightingGlobalLayoutCreateInfo, m_descriptorInfos[_deferred_lighting].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create deferred lighting global layout");
		}
	}
}

void MainCameraPass::SetupPipelines()
{
	m_renderPipelines.resize(_render_pipeline_type_count);	// 调整渲染管线数组大小

	// 网格GBuffer 图形管线
	{
		RHIDescriptorSetLayout* descriptorsetLayouts[3] = {
			m_descriptorInfos[_mesh_global].m_pDescriptorSetLayout,
			m_descriptorInfos[_per_mesh].m_pDescriptorSetLayout,
			m_descriptorInfos[_mesh_per_material].m_pDescriptorSetLayout
		};

		// 创建渲染管线布局
		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = 3;
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh gbuffer pipeline layout");
		}

		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(MESH_VERT);	// 顶点着色器
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(MESH_GBUFFER_FRAG);	// 片段着色器

		// 顶点着色器阶段创建信息
		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";

		// 片段着色器阶段创建信息
		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo shaderStages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		// 获取顶点布局信息
		auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptions();
		auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptions();

		// 顶点输入状态创建信息（顶点布局）
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = vertexBindingDescriptions.size();
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = &vertexBindingDescriptions[0];
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

		// 输入汇编状态创建信息（拓扑类型）
		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = ERHIPrimitiveTopology::RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	// 图元拓扑类型
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;	// 是否启用图元重启

		// 视口状态创建信息（视口和裁剪矩形）
		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		// 光栅化状态创建信息
		ST_RHIPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info.m_depthClampEnable = RHI_FALSE;
		rasterization_state_create_info.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterization_state_create_info.m_polygonMode = ERHIPolygonMode::RHI_POLYGON_MODE_FILL;
		rasterization_state_create_info.m_lineWidth = 1.0f;
		rasterization_state_create_info.m_cullMode = ERHICullModeFlagBits::RHI_CULL_MODE_BACK_BIT;	// 背面剔除
		rasterization_state_create_info.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_COUNTER_CLOCKWISE;	// 逆时针为正面
		rasterization_state_create_info.m_depthBiasEnable = RHI_FALSE;
		rasterization_state_create_info.m_depthBiasConstantFactor = 0.0f;
		rasterization_state_create_info.m_depthBiasClamp = 0.0f;
		rasterization_state_create_info.m_depthBiasSlopeFactor = 0.0f;

		// 多重采样状态创建信息
		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;
	}
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

