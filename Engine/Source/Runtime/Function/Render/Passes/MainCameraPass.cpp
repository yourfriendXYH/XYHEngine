#include "MainCameraPass.h"

#include <map>
#include <assert.h>
#include <MeshVert.h>
#include <MeshFrag.h>
#include <MeshGBufferFrag.h>
#include <DeferredLightingVert.h>
#include <DeferredLightingFrag.h>
#include <SkyboxVert.h>
#include <SkyboxFrag.h>
#include <AxisVert.h>
#include <AxisFrag.h>
#include "Runtime/Function/Render/RenderHelper.h"
#include "Runtime/Function/Render/RenderMesh.h"
#include "Runtime/Function/Render/Interface/Vulkan/VulkanRHIResource.h"

NAMESPACE_XYH_BEGIN

void MainCameraPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);	// ???

	const ST_MainCameraPassInitInfo* mainCameraPassInitInfo = static_cast<const ST_MainCameraPassInitInfo*>(initInfo);
	m_enableFXAA = mainCameraPassInitInfo->m_enableFXAA;

	SetupAttachments();	// 创建帧缓冲的图像及视图

	SetupRenderPass();	// 创建渲染通道

	SetupDescriptorSetLayout();	// 创建描述符集布局(色器的缓存数据)

	SetupPipelines();	// 创建渲染管线

	SetupDescriptorSet();	// 设置描述符集

	SetupFramebufferDescriptorSet();	// 创建帧缓冲描述符集

	SetupSwapchainFramebuffers();	// 创建交换链帧缓冲

	SetupParticlePass();	// 给粒子渲染通道设置需要用到的资源
}

void MainCameraPass::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
{
	const RenderResource* vulkanResource = static_cast<const RenderResource*>(renderResource.get());
	if (nullptr != vulkanResource)
	{
		m_meshPerframeStorageBufferObject = vulkanResource->m_meshPerframeStorageBufferObject;
		m_axisStorageBufferObject = vulkanResource->m_axisStorageBufferObject;
	}
}

void MainCameraPass::DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex)
{
}

void MainCameraPass::Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex)
{
	// 延迟渲染流程

	// 开始渲染通道
	{
		ST_RHIRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.m_sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		renderPassBeginInfo.m_pFramebuffer = m_swapchainFramebuffers[currentSwapchainImageIndex];
		renderPassBeginInfo.m_renderArea.m_offset = { 0, 0 };
		renderPassBeginInfo.m_renderArea.m_extent = m_pRHI->GetSwapchainInfo().m_extent;

		// 附件清除值
		UN_RHIClearValue clearValues[_main_camera_pass_attachment_count];
		clearValues[_main_camera_pass_gbuffer_a].m_color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearValues[_main_camera_pass_gbuffer_b].m_color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearValues[_main_camera_pass_gbuffer_c].m_color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearValues[_main_camera_pass_backup_buffer_odd].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[_main_camera_pass_backup_buffer_even].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[_main_camera_pass_post_process_buffer_odd].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[_main_camera_pass_post_process_buffer_even].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[_main_camera_pass_depth].m_depthStencil = { 1.0f, 0 };
		clearValues[_main_camera_pass_swap_chain_image].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		renderPassBeginInfo.m_clearValueCount = (sizeof(clearValues) / sizeof(clearValues[0]));
		renderPassBeginInfo.m_pClearValues = clearValues;

		m_pRHI->CmdBeginRenderPassPFN(m_pRHI->GetCurrentCommandBuffer(), &renderPassBeginInfo, RHI_SUBPASS_CONTENTS_INLINE);
	}

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "BasePass", color);	// 开启调试标签
	// 绘制网格的GBuffer数据，绘制到帧缓冲的前三个颜色附件上（a、b、c）
	// 对应到第一个子通道_main_camera_subpass_basepass的颜色附件
	DrawMeshGbuffer();	// 绘制网格GBuffer
	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());	// 结束调试标签

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);	// 切换到下一个子通道
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Deferred Lighting", color);
	// 根据颜色附件和光源数据将光照计算结果绘制到帧缓冲的第四个颜色附件上（_main_camera_pass_backup_buffer_odd）偶数备用缓冲区
	DrawDeferredLighting();	// 绘制延迟光照
	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Forward Lighting", color);
	particlePass.Draw();	// 绘制粒子
	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	toneMappingPass.Draw();	// 色调映射

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	colorGradingPass.Draw();	// 颜色分级

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	if (m_enableFXAA)
		fxaaPass.Draw();	// FXAA抗锯齿

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	ST_RHIClearAttachment clearAttachments[1];
	clearAttachments[0].m_aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;	// 清除颜色附件
	clearAttachments[0].m_colorAttachment = 0;	// 第0个颜色附件
	clearAttachments[0].m_clearValue.m_color.m_float32[0] = 0.0;
	clearAttachments[0].m_clearValue.m_color.m_float32[1] = 0.0;
	clearAttachments[0].m_clearValue.m_color.m_float32[2] = 0.0;
	clearAttachments[0].m_clearValue.m_color.m_float32[3] = 0.0;
	ST_RHIClearRect clearRects[1];
	clearRects[0].m_baseArrayLayer = 0;
	clearRects[0].m_layerCount = 1;
	clearRects[0].m_rect.m_offset.m_x = 0;
	clearRects[0].m_rect.m_offset.m_y = 0;
	clearRects[0].m_rect.m_extent.m_width = m_pRHI->GetSwapchainInfo().m_extent.m_width;
	clearRects[0].m_rect.m_extent.m_height = m_pRHI->GetSwapchainInfo().m_extent.m_height;
	// 清除为 (0, 0, 0, 0) 表示完全透明的黑色
	// 目的：在已有3D场景上创建透明层，用于绘制Axis和UI
	// [已有的3D场景内容]  // ← 保持可见
	// [透明清除区域]      // ← RGBA(0,0,0,0) - 完全透明
	// [Axis和UI绘制]      // ← 在透明背景上绘制
	m_pRHI->CmdClearAttachmentsPFN(m_pRHI->GetCurrentCommandBuffer(), sizeof(clearAttachments) / sizeof(clearAttachments[0]), clearAttachments, sizeof(clearRects) / sizeof(clearRects[0]), clearRects);
	DrawAxis();	// 绘制坐标轴
	uiPass.Draw();	// 绘制UI

	m_pRHI->CmdNextSubpassPFN(m_pRHI->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);
	combineUIPass.Draw();	// 合并UI

	m_pRHI->CmdEndRenderPassPFN(m_pRHI->GetCurrentCommandBuffer());
}

void MainCameraPass::SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass)
{
	m_pParticlePass = pParticlePass;
}

RHICommandBuffer* MainCameraPass::GetRenderCommandBuffer()
{
	return m_pRHI->GetCurrentCommandBuffer();
}

void MainCameraPass::UpdateAfterFramebufferRecreate()
{
	for (size_t i = 0; i < m_framebuffer.m_attachments.size(); i++)
	{
		m_pRHI->DestroyImage(m_framebuffer.m_attachments[i].m_pImage);
		m_pRHI->DestroyImageView(m_framebuffer.m_attachments[i].m_pView);
		m_pRHI->FreeMemory(m_framebuffer.m_attachments[i].m_pMemory);
	}

	for (auto framebuffer : m_swapchainFramebuffers)
	{
		m_pRHI->DestroyFramebuffer(framebuffer);
	}

	// 设置附件
	SetupAttachments();

	// 设置帧缓冲描述符集
	SetupFramebufferDescriptorSet();

	// 设置交换链帧缓冲
	SetupSwapchainFramebuffers();

	SetupParticlePass();
}

void MainCameraPass::SetupParticlePass()
{
	// 需要深度图像和法线图像
	m_pParticlePass->SetDepthAndNormalImage(m_pRHI->GetDepthImageInfo().m_depthImage, m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_pImage);
	// 需要RenderPass
	m_pParticlePass->SetRenderPassHandle(m_framebuffer.m_pRenderPass);
}

void MainCameraPass::SetupAttachments()
{
	m_framebuffer.m_attachments.resize(_main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count);	// 5 + 2 = 7

	m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;	// GBuffer A: 法线
	m_framebuffer.m_attachments[_main_camera_pass_gbuffer_b].m_format = ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM;	// GBuffer B: 金属粗糙度
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
		ST_RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerFrameStorageBufferBinding = meshGlobalLayoutBindings[0];
		meshGlobalLayoutPerFrameStorageBufferBinding.m_binding = 0;
		meshGlobalLayoutPerFrameStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		meshGlobalLayoutPerFrameStorageBufferBinding.m_descriptorCount = 1;
		meshGlobalLayoutPerFrameStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
		meshGlobalLayoutPerFrameStorageBufferBinding.m_pImmutableSamplers = nullptr;

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
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = &vertexBindingDescriptions[0];
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
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
		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = ERHIPolygonMode::RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = ERHICullModeFlagBits::RHI_CULL_MODE_BACK_BIT;	// 背面剔除
		rasterizationStateCreateInfo.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_COUNTER_CLOCKWISE;	// 逆时针为正面
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		// 多重采样状态创建信息
		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;

		// 每个颜色附件的混合状态
		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[3] = {};	// 每个颜色附件的混合状态
		colorBlendAttachments[0].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[0].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[0].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
		colorBlendAttachments[0].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[0].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

		colorBlendAttachments[1].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[1].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[1].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[1].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[1].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
		colorBlendAttachments[1].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[1].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[1].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

		colorBlendAttachments[2].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[2].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[2].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[2].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[2].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
		colorBlendAttachments[2].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[2].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[2].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

		// 颜色混合状态创建信息
		ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;
		colorBlendStateCreateInfo.m_logicOp = RHI_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.m_attachmentCount = sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
		colorBlendStateCreateInfo.m_pAttachments = &colorBlendAttachments[0];
		colorBlendStateCreateInfo.m_blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[3] = 0.0f;

		// 深度模板状态创建信息
		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthCompareOp = ERHICompareOp::RHI_COMPARE_OP_LESS;	// 通过深度测试的条件
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		// 动态状态创建信息
		ERHIDynamicState dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
		ST_RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.m_dynamicStateCount = 2;
		dynamicStateCreateInfo.m_pDynamicStates = dynamicStates;

		// 图形管线创建信息
		ST_RHIGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.m_sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.m_stageCount = 2;
		pipelineInfo.m_pStages = shaderStages;	// 着色器阶段
		pipelineInfo.m_pVertexInputState = &vertexInputStateCreateInfo;
		pipelineInfo.m_pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineInfo.m_pViewportState = &viewportStateCreateInfo;
		pipelineInfo.m_pRasterizationState = &rasterizationStateCreateInfo;
		pipelineInfo.m_pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.m_pColorBlendState = &colorBlendStateCreateInfo;
		pipelineInfo.m_pDepthStencilState = &depthStencilCreateInfo;
		pipelineInfo.m_pLayout = m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_basepass;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		if (RHI_SUCCESS != m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipeline))
		{
			throw std::runtime_error("create mesh gbuffer graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}

	// 延迟照明 图形管线
	{
		RHIDescriptorSetLayout* descriptorsetLayouts[3] = {
			m_descriptorInfos[_mesh_global].m_pDescriptorSetLayout,
			m_descriptorInfos[_deferred_lighting].m_pDescriptorSetLayout,
			m_descriptorInfos[_skybox].m_pDescriptorSetLayout
		};

		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = sizeof(descriptorsetLayouts) / sizeof(descriptorsetLayouts[0]);
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		// 着色器常数缓存布局
		if (RHI_SUCCESS != m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[_render_pipeline_type_deferred_lighting].m_pipelineLayout))
		{
			throw std::runtime_error("create deferred lighting pipeline layout");
		}

		// 延迟照明着色器
		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(DEFERRED_LIGHTING_VERT);
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(DEFERRED_LIGHTING_FRAG);

		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";
		// vert_pipeline_shader_stage_create_info.pSpecializationInfo

		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		// 着色器阶段创建信息数组
		ST_RHIPipelineShaderStageCreateInfo shader_stages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		// 顶点输入状态创建信息
		auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptions();
		auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptions();
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = NULL;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = NULL;

		// 输入汇编状态创建信息
		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = ERHIPrimitiveTopology::RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;

		// 视口状态创建信息
		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		// 光栅化状态创建信息
		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;	// 是否启用深度夹具
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = ERHIPolygonMode::RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = ERHICullModeFlagBits::RHI_CULL_MODE_BACK_BIT;
		rasterizationStateCreateInfo.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_CLOCKWISE;
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		// 多重采样状态创建信息
		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

		// 颜色混合状态创建信息
		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
		colorBlendAttachments[0].m_colorWriteMask = ERHIColorComponentFlagBits::RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[0].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
		colorBlendAttachments[0].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

		// 颜色混合状态创建信息
		ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;
		colorBlendStateCreateInfo.m_logicOp = ERHILogicOp::RHI_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.m_attachmentCount = sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
		colorBlendStateCreateInfo.m_pAttachments = &colorBlendAttachments[0];
		colorBlendStateCreateInfo.m_blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[3] = 0.0f;

		// 深度模板状态创建信息
		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthCompareOp = ERHICompareOp::RHI_COMPARE_OP_LESS;
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		// 动态状态创建信息
		ERHIDynamicState dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
		ST_RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.m_dynamicStateCount = 2;
		dynamicStateCreateInfo.m_pDynamicStates = dynamicStates;

		// 图形管线创建信息
		ST_RHIGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.m_sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.m_stageCount = 2;
		pipelineInfo.m_pStages = shader_stages;
		pipelineInfo.m_pVertexInputState = &vertexInputStateCreateInfo;
		pipelineInfo.m_pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineInfo.m_pViewportState = &viewportStateCreateInfo;
		pipelineInfo.m_pRasterizationState = &rasterizationStateCreateInfo;
		pipelineInfo.m_pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.m_pColorBlendState = &colorBlendStateCreateInfo;
		pipelineInfo.m_pDepthStencilState = &depthStencilCreateInfo;
		pipelineInfo.m_pLayout = m_renderPipelines[_render_pipeline_type_deferred_lighting].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_deferred_lighting;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		if (m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[_render_pipeline_type_deferred_lighting].m_pipeline) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh lighting graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}

	// 网格光照 图形管线
	{
		RHIDescriptorSetLayout* descriptorsetLayouts[3] = {
			m_descriptorInfos[_mesh_global].m_pDescriptorSetLayout,
			m_descriptorInfos[_per_mesh].m_pDescriptorSetLayout,
			m_descriptorInfos[_mesh_per_material].m_pDescriptorSetLayout
		};
		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = 3;
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[_render_pipeline_type_mesh_lighting].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh lighting pipeline layout");
		}

		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(MESH_VERT);
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(MESH_FRAG);

		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = RHI_SHADER_STAGE_VERTEX_BIT;
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo shaderStages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptions();
		auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptions();
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = &vertexBindingDescriptions[0];
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;

		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = RHI_CULL_MODE_BACK_BIT;
		rasterizationStateCreateInfo.m_frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
		colorBlendAttachments[0].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[0].m_srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstColorBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_colorBlendOp = RHI_BLEND_OP_ADD;
		colorBlendAttachments[0].m_srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_alphaBlendOp = RHI_BLEND_OP_ADD;

		ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;
		colorBlendStateCreateInfo.m_logicOp = RHI_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.m_attachmentCount = sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
		colorBlendStateCreateInfo.m_pAttachments = &colorBlendAttachments[0];
		colorBlendStateCreateInfo.m_blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[3] = 0.0f;

		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthCompareOp = RHI_COMPARE_OP_LESS;
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		ERHIDynamicState dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
		ST_RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.m_dynamicStateCount = 2;
		dynamicStateCreateInfo.m_pDynamicStates = dynamicStates;

		ST_RHIGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.m_sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.m_stageCount = 2;
		pipelineInfo.m_pStages = shaderStages;
		pipelineInfo.m_pVertexInputState = &vertexInputStateCreateInfo;
		pipelineInfo.m_pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineInfo.m_pViewportState = &viewportStateCreateInfo;
		pipelineInfo.m_pRasterizationState = &rasterizationStateCreateInfo;
		pipelineInfo.m_pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.m_pColorBlendState = &colorBlendStateCreateInfo;
		pipelineInfo.m_pDepthStencilState = &depthStencilCreateInfo;
		pipelineInfo.m_pLayout = m_renderPipelines[_render_pipeline_type_mesh_lighting].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_forward_lighting;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		if (m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[_render_pipeline_type_mesh_lighting].m_pipeline) != RHI_SUCCESS)
		{
			throw std::runtime_error("create mesh lighting graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}

	// 天空盒
	{
		// 创建 天空盒描述符集 布局
		RHIDescriptorSetLayout* descriptorsetLayouts[1] = { m_descriptorInfos[_skybox].m_pDescriptorSetLayout };
		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = 1;
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		// 创建 渲染管线布局
		if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[_render_pipeline_type_skybox].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create skybox pipeline layout");
		}

		// 着色器阶段 创建信息
		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(SKYBOX_VERT);
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(SKYBOX_FRAG);

		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";
		// vert_pipeline_shader_stage_create_info.pSpecializationInfo

		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo shader_stages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		// 顶点输入状态创建信息
		auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptions();
		auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptions();
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = nullptr;
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = nullptr;

		// 输入汇编状态创建信息
		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = ERHIPrimitiveTopology::RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;

		// 视口状态创建信息
		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		// 光栅化状态创建信息
		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = ERHIPolygonMode::RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = ERHICullModeFlagBits::RHI_CULL_MODE_BACK_BIT;
		rasterizationStateCreateInfo.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		// 多重采样状态创建信息
		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;

		// 颜色混合状态创建信息
		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
		colorBlendAttachments[0].m_colorWriteMask = ERHIColorComponentFlagBits::RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].m_blendEnable = RHI_FALSE;
		colorBlendAttachments[0].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[0].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
		colorBlendAttachments[0].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachments[0].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

		ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;
		colorBlendStateCreateInfo.m_logicOp = RHI_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.m_attachmentCount = sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
		colorBlendStateCreateInfo.m_pAttachments = &colorBlendAttachments[0];
		colorBlendStateCreateInfo.m_blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[3] = 0.0f;

		// 深度模板状态创建信息
		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthCompareOp = ERHICompareOp::RHI_COMPARE_OP_LESS;
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		// 动态状态创建信息
		ERHIDynamicState dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
		ST_RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.m_dynamicStateCount = 2;
		dynamicStateCreateInfo.m_pDynamicStates = dynamicStates;

		// 图形管线创建信息
		ST_RHIGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.m_sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.m_stageCount = 2;
		pipelineInfo.m_pStages = shader_stages;
		pipelineInfo.m_pVertexInputState = &vertexInputStateCreateInfo;
		pipelineInfo.m_pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineInfo.m_pViewportState = &viewportStateCreateInfo;
		pipelineInfo.m_pRasterizationState = &rasterizationStateCreateInfo;
		pipelineInfo.m_pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.m_pColorBlendState = &colorBlendStateCreateInfo;
		pipelineInfo.m_pDepthStencilState = &depthStencilCreateInfo;
		pipelineInfo.m_pLayout = m_renderPipelines[_render_pipeline_type_skybox].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_forward_lighting;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		// 创建 图形管线
		if (RHI_SUCCESS != m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[_render_pipeline_type_skybox].m_pipeline))
		{
			throw std::runtime_error("create skybox graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}

	// 坐标轴
	{
		RHIDescriptorSetLayout* descriptorsetLayouts[1] = { m_descriptorInfos[_axis].m_pDescriptorSetLayout };
		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = 1;
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[_render_pipeline_type_axis].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create axis pipeline layout");
		}

		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(AXIS_VERT);
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(AXIS_FRAG);

		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";
		// vert_pipeline_shader_stage_create_info.pSpecializationInfo

		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo shaderStages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptions();
		auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptions();
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = &vertexBindingDescriptions[0];
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = ERHIPrimitiveTopology::RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;

		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = ERHIPolygonMode::RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = RHI_CULL_MODE_NONE;
		rasterizationStateCreateInfo.m_frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachmentState{};
		colorBlendAttachmentState.m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.m_blendEnable = RHI_FALSE;
		colorBlendAttachmentState.m_srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.m_dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.m_colorBlendOp = RHI_BLEND_OP_ADD;
		colorBlendAttachmentState.m_srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.m_dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.m_alphaBlendOp = RHI_BLEND_OP_ADD;

		ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
		colorBlendStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;
		colorBlendStateCreateInfo.m_logicOp = RHI_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.m_attachmentCount = 1;
		colorBlendStateCreateInfo.m_pAttachments = &colorBlendAttachmentState;
		colorBlendStateCreateInfo.m_blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.m_blendConstants[3] = 0.0f;

		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_FALSE;
		depthStencilCreateInfo.m_depthCompareOp = RHI_COMPARE_OP_LESS;
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		ERHIDynamicState dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
		ST_RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.m_dynamicStateCount = 2;
		dynamicStateCreateInfo.m_pDynamicStates = dynamicStates;

		ST_RHIGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.m_sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.m_stageCount = 2;
		pipelineInfo.m_pStages = shaderStages;
		pipelineInfo.m_pVertexInputState = &vertexInputStateCreateInfo;
		pipelineInfo.m_pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineInfo.m_pViewportState = &viewportStateCreateInfo;
		pipelineInfo.m_pRasterizationState = &rasterizationStateCreateInfo;
		pipelineInfo.m_pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.m_pColorBlendState = &colorBlendStateCreateInfo;
		pipelineInfo.m_pDepthStencilState = &depthStencilCreateInfo;
		pipelineInfo.m_pLayout = m_renderPipelines[_render_pipeline_type_axis].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_ui;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		if (RHI_SUCCESS != m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[_render_pipeline_type_axis].m_pipeline))
		{
			throw std::runtime_error("create axis graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}
}

void MainCameraPass::SetupDescriptorSet()
{
	SetupModelGlobalDescriptorSet();
	SetupSkyboxDescriptorSet();
	SetupAxisDescriptorSet();
	SetupGbufferLightingDescriptorSet();
}

void MainCameraPass::SetupFramebufferDescriptorSet()
{
	ST_RHIDescriptorImageInfo gbufferNormalInputAttachmentInfo = {};
	gbufferNormalInputAttachmentInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	gbufferNormalInputAttachmentInfo.m_pImageView = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_pView;
	gbufferNormalInputAttachmentInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo gbufferMetallicRoughnessShadingmodeidInputAttachmentInfo = {};
	gbufferMetallicRoughnessShadingmodeidInputAttachmentInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	gbufferMetallicRoughnessShadingmodeidInputAttachmentInfo.m_pImageView = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_b].m_pView;
	gbufferMetallicRoughnessShadingmodeidInputAttachmentInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo gbufferAlbedoInputAttachmentInfo = {};
	gbufferAlbedoInputAttachmentInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	gbufferAlbedoInputAttachmentInfo.m_pImageView = m_framebuffer.m_attachments[_main_camera_pass_gbuffer_c].m_pView;
	gbufferAlbedoInputAttachmentInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo depthInputAttachmentInfo = {};
	depthInputAttachmentInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	depthInputAttachmentInfo.m_pImageView = m_pRHI->GetDepthImageInfo().m_depthImageView;
	depthInputAttachmentInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIWriteDescriptorSet deferredLightingDescriptorWritesInfo[4];

	// 位置法线贴图 输入附件
	ST_RHIWriteDescriptorSet& gbufferNormalDescriptorInputAttachmenWriteInfo = deferredLightingDescriptorWritesInfo[0];
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_pNext = nullptr;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_pDstSet = m_descriptorInfos[_deferred_lighting].m_pDescriptorSet;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_dstBinding = 0;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_dstArrayElement = 0;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_descriptorCount = 1;
	gbufferNormalDescriptorInputAttachmenWriteInfo.m_pImageInfo = &gbufferNormalInputAttachmentInfo;

	// 金属度粗糙度阴影模式id贴图 输入附件
	ST_RHIWriteDescriptorSet& gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo = deferredLightingDescriptorWritesInfo[1];
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_pNext = nullptr;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_pDstSet = m_descriptorInfos[_deferred_lighting].m_pDescriptorSet;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_dstBinding = 1;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_dstArrayElement = 0;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_descriptorCount = 1;
	gbufferMetallicRoughnessShadingmodeidDescriptorInputAttachmentWriteInfo.m_pImageInfo = &gbufferMetallicRoughnessShadingmodeidInputAttachmentInfo;

	// 反射率贴图 输入附件
	ST_RHIWriteDescriptorSet& gbufferAlbedoDescriptorInputAttachmentWriteInfo = deferredLightingDescriptorWritesInfo[2];
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_pNext = nullptr;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_pDstSet = m_descriptorInfos[_deferred_lighting].m_pDescriptorSet;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_dstBinding = 2;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_dstArrayElement = 0;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_descriptorCount = 1;
	gbufferAlbedoDescriptorInputAttachmentWriteInfo.m_pImageInfo = &gbufferAlbedoInputAttachmentInfo;

	// 深度贴图 输入附件
	ST_RHIWriteDescriptorSet& depthDescriptorInputAttachmentWriteInfo = deferredLightingDescriptorWritesInfo[3];
	depthDescriptorInputAttachmentWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	depthDescriptorInputAttachmentWriteInfo.m_pNext = nullptr;
	depthDescriptorInputAttachmentWriteInfo.m_pDstSet = m_descriptorInfos[_deferred_lighting].m_pDescriptorSet;
	depthDescriptorInputAttachmentWriteInfo.m_dstBinding = 3;
	depthDescriptorInputAttachmentWriteInfo.m_dstArrayElement = 0;
	depthDescriptorInputAttachmentWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthDescriptorInputAttachmentWriteInfo.m_descriptorCount = 1;
	depthDescriptorInputAttachmentWriteInfo.m_pImageInfo = &depthInputAttachmentInfo;

	// 更新描述符集
	m_pRHI->UpdateDescriptorSets(sizeof(deferredLightingDescriptorWritesInfo) / sizeof(deferredLightingDescriptorWritesInfo[0]), deferredLightingDescriptorWritesInfo, 0, nullptr);
}

void MainCameraPass::SetupSwapchainFramebuffers()
{
	m_swapchainFramebuffers.resize(m_pRHI->GetSwapchainInfo().m_imageViews.size());

	// 为交换链的每个图像视图创建一个帧缓冲区
	// 3缓冲渲染
	for (size_t i = 0; i < m_pRHI->GetSwapchainInfo().m_imageViews.size(); i++)	// 目前是3个帧缓冲
	{
		RHIImageView* framebufferAttachmentsForImageView[_main_camera_pass_attachment_count] = {
			m_framebuffer.m_attachments[_main_camera_pass_gbuffer_a].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_gbuffer_b].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_gbuffer_c].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_odd].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_backup_buffer_even].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_odd].m_pView,
			m_framebuffer.m_attachments[_main_camera_pass_post_process_buffer_even].m_pView,
			m_pRHI->GetDepthImageInfo().m_depthImageView,
			m_pRHI->GetSwapchainInfo().m_imageViews[i]
		};

		ST_RHIFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.m_sType = RHI_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.m_flags = 0U;
		framebufferCreateInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		framebufferCreateInfo.m_attachmentCount = (sizeof(framebufferAttachmentsForImageView) / sizeof(framebufferAttachmentsForImageView[0]));
		framebufferCreateInfo.m_pAttachments = framebufferAttachmentsForImageView;
		framebufferCreateInfo.m_width = m_pRHI->GetSwapchainInfo().m_extent.m_width;
		framebufferCreateInfo.m_height = m_pRHI->GetSwapchainInfo().m_extent.m_height;
		framebufferCreateInfo.m_layers = 1;

		m_swapchainFramebuffers[i] = new VulkanFramebuffer();
		if (RHI_SUCCESS != m_pRHI->CreateFramebuffer(&framebufferCreateInfo, m_swapchainFramebuffers[i]))
		{
			throw std::runtime_error("create main camera framebuffer");
		}
	}
}

void MainCameraPass::SetupModelGlobalDescriptorSet()
{
	ST_RHIDescriptorSetAllocateInfo meshGlobalDescriptorSetAllocInfo;
	meshGlobalDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	meshGlobalDescriptorSetAllocInfo.m_pNext = NULL;
	meshGlobalDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
	meshGlobalDescriptorSetAllocInfo.m_descriptorSetCount = 1;
	meshGlobalDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[_mesh_global].m_pDescriptorSetLayout;
	// 分配 描述符集
	if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&meshGlobalDescriptorSetAllocInfo, m_descriptorInfos[_mesh_global].m_pDescriptorSet))
	{
		throw std::runtime_error("allocate mesh global descriptor set");
	}

	ST_RHIDescriptorBufferInfo meshPerframeStorageBufferInfo = {};
	meshPerframeStorageBufferInfo.m_offset = 0;	// 这个偏移量加上dynamic_offset不应该大于缓冲区的大小
	meshPerframeStorageBufferInfo.m_range = sizeof(ST_MeshPerframeStorageBufferObject);	// 范围是指每次绘制调用时着色器实际使用的大小
	meshPerframeStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbuffer;
	assert(meshPerframeStorageBufferInfo.m_range < m_pGlobalRenderResource->m_storageBuffer.m_maxStorageBufferRange);

	ST_RHIDescriptorBufferInfo meshPerdrawcallStorageBufferInfo = {};
	meshPerdrawcallStorageBufferInfo.m_offset = 0;
	meshPerdrawcallStorageBufferInfo.m_range = sizeof(ST_MeshPerdrawcallStorageBufferObject);
	meshPerdrawcallStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbuffer;
	assert(meshPerdrawcallStorageBufferInfo.m_range < m_pGlobalRenderResource->m_storageBuffer.m_maxStorageBufferRange);

	ST_RHIDescriptorBufferInfo meshPerDrawcallVertexBlendingStorageBufferInfo = {};
	meshPerDrawcallVertexBlendingStorageBufferInfo.m_offset = 0;
	meshPerDrawcallVertexBlendingStorageBufferInfo.m_range = sizeof(ST_MeshPerdrawcallVertexBlendingStorageBufferObject);
	meshPerDrawcallVertexBlendingStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbuffer;
	assert(meshPerDrawcallVertexBlendingStorageBufferInfo.m_range < m_pGlobalRenderResource->m_storageBuffer.m_maxStorageBufferRange);

	ST_RHIDescriptorImageInfo brdfTextureImageInfo = {};
	brdfTextureImageInfo.m_pSampler = m_pGlobalRenderResource->m_iblResource.m_pBrdfLUTTextureSampler;
	brdfTextureImageInfo.m_pImageView = m_pGlobalRenderResource->m_iblResource.m_pBrdfLUTTextureImageView;
	brdfTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo irradianceTextureImageInfo = {};
	irradianceTextureImageInfo.m_pSampler = m_pGlobalRenderResource->m_iblResource.m_pIrradianceTextureSampler;
	irradianceTextureImageInfo.m_pImageView = m_pGlobalRenderResource->m_iblResource.m_pIrradianceTextureImageView;
	irradianceTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo specularTextureImageInfo{};
	specularTextureImageInfo.m_pSampler = m_pGlobalRenderResource->m_iblResource.m_pSpecularTextureSampler;
	specularTextureImageInfo.m_pImageView = m_pGlobalRenderResource->m_iblResource.m_pSpecularTextureImageView;
	specularTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo pointLightShadowTextureImageInfo{};
	pointLightShadowTextureImageInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	pointLightShadowTextureImageInfo.m_pImageView = m_pPointLightShadowColorImageView;
	pointLightShadowTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	ST_RHIDescriptorImageInfo directionalLightShadowTextureImageInfo{};
	directionalLightShadowTextureImageInfo.m_pSampler = m_pRHI->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
	directionalLightShadowTextureImageInfo.m_pImageView = m_pDirectionalLightShadowColorImageView;
	directionalLightShadowTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	ST_RHIWriteDescriptorSet meshDescriptorWritesInfo[8];

	meshDescriptorWritesInfo[0].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	meshDescriptorWritesInfo[0].m_pNext = NULL;
	meshDescriptorWritesInfo[0].m_pDstSet = m_descriptorInfos[_mesh_global].m_pDescriptorSet;
	meshDescriptorWritesInfo[0].m_dstBinding = 0;
	meshDescriptorWritesInfo[0].m_dstArrayElement = 0;
	meshDescriptorWritesInfo[0].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	meshDescriptorWritesInfo[0].m_descriptorCount = 1;
	meshDescriptorWritesInfo[0].m_pBufferInfo = &meshPerframeStorageBufferInfo;

	meshDescriptorWritesInfo[1].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	meshDescriptorWritesInfo[1].m_pNext = NULL;
	meshDescriptorWritesInfo[1].m_pDstSet = m_descriptorInfos[_mesh_global].m_pDescriptorSet;
	meshDescriptorWritesInfo[1].m_dstBinding = 1;
	meshDescriptorWritesInfo[1].m_dstArrayElement = 0;
	meshDescriptorWritesInfo[1].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	meshDescriptorWritesInfo[1].m_descriptorCount = 1;
	meshDescriptorWritesInfo[1].m_pBufferInfo = &meshPerdrawcallStorageBufferInfo;

	meshDescriptorWritesInfo[2].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	meshDescriptorWritesInfo[2].m_pNext = NULL;
	meshDescriptorWritesInfo[2].m_pDstSet = m_descriptorInfos[_mesh_global].m_pDescriptorSet;
	meshDescriptorWritesInfo[2].m_dstBinding = 2;
	meshDescriptorWritesInfo[2].m_dstArrayElement = 0;
	meshDescriptorWritesInfo[2].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	meshDescriptorWritesInfo[2].m_descriptorCount = 1;
	meshDescriptorWritesInfo[2].m_pBufferInfo = &meshPerDrawcallVertexBlendingStorageBufferInfo;

	meshDescriptorWritesInfo[3].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	meshDescriptorWritesInfo[3].m_pNext = NULL;
	meshDescriptorWritesInfo[3].m_pDstSet = m_descriptorInfos[_mesh_global].m_pDescriptorSet;
	meshDescriptorWritesInfo[3].m_dstBinding = 3;
	meshDescriptorWritesInfo[3].m_dstArrayElement = 0;
	meshDescriptorWritesInfo[3].m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	meshDescriptorWritesInfo[3].m_descriptorCount = 1;
	meshDescriptorWritesInfo[3].m_pImageInfo = &brdfTextureImageInfo;

	meshDescriptorWritesInfo[4] = meshDescriptorWritesInfo[3];
	meshDescriptorWritesInfo[4].m_dstBinding = 4;
	meshDescriptorWritesInfo[4].m_pImageInfo = &irradianceTextureImageInfo;

	meshDescriptorWritesInfo[5] = meshDescriptorWritesInfo[3];
	meshDescriptorWritesInfo[5].m_dstBinding = 5;
	meshDescriptorWritesInfo[5].m_pImageInfo = &specularTextureImageInfo;

	meshDescriptorWritesInfo[6] = meshDescriptorWritesInfo[3];
	meshDescriptorWritesInfo[6].m_dstBinding = 6;
	meshDescriptorWritesInfo[6].m_pImageInfo = &pointLightShadowTextureImageInfo;

	meshDescriptorWritesInfo[7] = meshDescriptorWritesInfo[3];
	meshDescriptorWritesInfo[7].m_dstBinding = 7;
	meshDescriptorWritesInfo[7].m_pImageInfo = &directionalLightShadowTextureImageInfo;

	m_pRHI->UpdateDescriptorSets(sizeof(meshDescriptorWritesInfo) / sizeof(meshDescriptorWritesInfo[0]), meshDescriptorWritesInfo, 0, nullptr);
}

void MainCameraPass::SetupSkyboxDescriptorSet()
{
	ST_RHIDescriptorSetAllocateInfo skyboxDescriptorSetAllocInfo;
	skyboxDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	skyboxDescriptorSetAllocInfo.m_pNext = NULL;
	skyboxDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
	skyboxDescriptorSetAllocInfo.m_descriptorSetCount = 1;
	skyboxDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[_skybox].m_pDescriptorSetLayout;

	if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&skyboxDescriptorSetAllocInfo, m_descriptorInfos[_skybox].m_pDescriptorSet))
	{
		throw std::runtime_error("allocate skybox descriptor set");
	}

	ST_RHIDescriptorBufferInfo meshPerframeStorageBufferInfo = {};
	meshPerframeStorageBufferInfo.m_offset = 0;
	meshPerframeStorageBufferInfo.m_range = sizeof(ST_MeshPerframeStorageBufferObject);
	meshPerframeStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbuffer;
	assert(meshPerframeStorageBufferInfo.m_range < m_pGlobalRenderResource->m_storageBuffer.m_maxStorageBufferRange);

	ST_RHIDescriptorImageInfo specularTextureImageInfo = {};
	specularTextureImageInfo.m_pSampler = m_pGlobalRenderResource->m_iblResource.m_pSpecularTextureSampler;
	specularTextureImageInfo.m_pImageView = m_pGlobalRenderResource->m_iblResource.m_pSpecularTextureImageView;
	specularTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	ST_RHIWriteDescriptorSet skyboxDescriptorWritesInfo[2];

	skyboxDescriptorWritesInfo[0].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	skyboxDescriptorWritesInfo[0].m_pNext = NULL;
	skyboxDescriptorWritesInfo[0].m_pDstSet = m_descriptorInfos[_skybox].m_pDescriptorSet;
	skyboxDescriptorWritesInfo[0].m_dstBinding = 0;
	skyboxDescriptorWritesInfo[0].m_dstArrayElement = 0;
	skyboxDescriptorWritesInfo[0].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	skyboxDescriptorWritesInfo[0].m_descriptorCount = 1;
	skyboxDescriptorWritesInfo[0].m_pBufferInfo = &meshPerframeStorageBufferInfo;

	skyboxDescriptorWritesInfo[1].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	skyboxDescriptorWritesInfo[1].m_pNext = NULL;
	skyboxDescriptorWritesInfo[1].m_pDstSet = m_descriptorInfos[_skybox].m_pDescriptorSet;
	skyboxDescriptorWritesInfo[1].m_dstBinding = 1;
	skyboxDescriptorWritesInfo[1].m_dstArrayElement = 0;
	skyboxDescriptorWritesInfo[1].m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	skyboxDescriptorWritesInfo[1].m_descriptorCount = 1;
	skyboxDescriptorWritesInfo[1].m_pImageInfo = &specularTextureImageInfo;

	m_pRHI->UpdateDescriptorSets(2, skyboxDescriptorWritesInfo, 0, nullptr);
}

void MainCameraPass::SetupAxisDescriptorSet()
{
	ST_RHIDescriptorSetAllocateInfo axisDescriptorSetAllocInfo;
	axisDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	axisDescriptorSetAllocInfo.m_pNext = NULL;
	axisDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
	axisDescriptorSetAllocInfo.m_descriptorSetCount = 1;
	axisDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[_axis].m_pDescriptorSetLayout;

	if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&axisDescriptorSetAllocInfo, m_descriptorInfos[_axis].m_pDescriptorSet))
	{
		throw std::runtime_error("allocate axis descriptor set");
	}

	ST_RHIDescriptorBufferInfo meshPerframeStorageBufferInfo = {};
	meshPerframeStorageBufferInfo.m_offset = 0;
	meshPerframeStorageBufferInfo.m_range = sizeof(ST_MeshPerframeStorageBufferObject);
	meshPerframeStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbuffer;
	assert(meshPerframeStorageBufferInfo.m_range < m_pGlobalRenderResource->m_storageBuffer.m_maxStorageBufferRange);

	ST_RHIDescriptorBufferInfo axisStorageBufferInfo = {};
	axisStorageBufferInfo.m_offset = 0;
	axisStorageBufferInfo.m_range = sizeof(ST_AxisStorageBufferObject);
	axisStorageBufferInfo.m_pBuffer = m_pGlobalRenderResource->m_storageBuffer.m_pAxisInefficientStorageBuffer;

	ST_RHIWriteDescriptorSet axisDescriptorWritesInfo[2];

	axisDescriptorWritesInfo[0].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	axisDescriptorWritesInfo[0].m_pNext = nullptr;
	axisDescriptorWritesInfo[0].m_pDstSet = m_descriptorInfos[_axis].m_pDescriptorSet;
	axisDescriptorWritesInfo[0].m_dstBinding = 0;
	axisDescriptorWritesInfo[0].m_dstArrayElement = 0;
	axisDescriptorWritesInfo[0].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	axisDescriptorWritesInfo[0].m_descriptorCount = 1;
	axisDescriptorWritesInfo[0].m_pBufferInfo = &meshPerframeStorageBufferInfo;

	axisDescriptorWritesInfo[1].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	axisDescriptorWritesInfo[1].m_pNext = nullptr;
	axisDescriptorWritesInfo[1].m_pDstSet = m_descriptorInfos[_axis].m_pDescriptorSet;
	axisDescriptorWritesInfo[1].m_dstBinding = 1;
	axisDescriptorWritesInfo[1].m_dstArrayElement = 0;
	axisDescriptorWritesInfo[1].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	axisDescriptorWritesInfo[1].m_descriptorCount = 1;
	axisDescriptorWritesInfo[1].m_pBufferInfo = &axisStorageBufferInfo;

	m_pRHI->UpdateDescriptorSets((uint32_t)(sizeof(axisDescriptorWritesInfo) / sizeof(axisDescriptorWritesInfo[0])), axisDescriptorWritesInfo, 0, nullptr);
}

void MainCameraPass::SetupGbufferLightingDescriptorSet()
{
	ST_RHIDescriptorSetAllocateInfo gbufferLightGlobalDescriptorSetAllocInfo;
	gbufferLightGlobalDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	gbufferLightGlobalDescriptorSetAllocInfo.m_pNext = NULL;
	gbufferLightGlobalDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
	gbufferLightGlobalDescriptorSetAllocInfo.m_descriptorSetCount = 1;
	gbufferLightGlobalDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[_deferred_lighting].m_pDescriptorSetLayout;

	if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&gbufferLightGlobalDescriptorSetAllocInfo, m_descriptorInfos[_deferred_lighting].m_pDescriptorSet))
	{
		throw std::runtime_error("allocate gbuffer light global descriptor set");
	}
}

void MainCameraPass::DrawMeshGbuffer()
{
	struct ST_MeshNode	// 网格节点
	{
		const Matrix4x4* m_modelMatrix{ nullptr };	// 模型矩阵
		const Matrix4x4* m_jointMatrices{ nullptr };	// 骨骼矩阵
		uint32_t m_jointCount{ 0 };	// 骨骼数量
	};

	std::map<ST_VulkanPBRMaterial*, std::map<ST_VulkanMesh*, std::vector<ST_MeshNode>>> mainCameraMeshDrawcallBatch;	// 主摄像机网格绘制调用批次

	for (ST_RenderMeshNode& node : *(s_visibleNodes.m_pMainCameraVisibleMeshNodes))	// 读取相机可见的网格节点
	{
		auto& meshInstanced = mainCameraMeshDrawcallBatch[node.m_refMaterial];
		auto& meshNodes = meshInstanced[node.m_refMesh];

		ST_MeshNode temp;
		temp.m_modelMatrix = node.m_modelMatrix;
		if (node.m_enableVertexBlending)
		{
			temp.m_jointMatrices = node.m_jointMatrices;
			temp.m_jointCount = node.m_jointCount;
		}

		meshNodes.push_back(temp);
	}

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Mesh GBuffer", color);	// 开始

	m_pRHI->CmdBindPipelinePFN(m_pRHI->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipeline);
	m_pRHI->CmdSetViewportPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pViewport);
	m_pRHI->CmdSetScissorPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pScissor);

	uint32_t perframeDynamicOffset = RoundUp(
		m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
		m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
	);

	m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] = perframeDynamicOffset + sizeof(ST_MeshPerframeStorageBufferObject);

	assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
		(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
			m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

	(*reinterpret_cast<ST_MeshPerframeStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perframeDynamicOffset)) = m_meshPerframeStorageBufferObject;

	for (auto& pair1 : mainCameraMeshDrawcallBatch)	// 根据材质遍历
	{
		ST_VulkanPBRMaterial& material = (*pair1.first);
		auto& meshInstanced = pair1.second;

		// 绑定材质的描述符集
		m_pRHI->CmdBindDescriptorSetsPFN(
			m_pRHI->GetCurrentCommandBuffer(),
			RHI_PIPELINE_BIND_POINT_GRAPHICS,
			m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipelineLayout,	// 管线布局
			2,	// 绑定到哪一个描述符集布局上（例如_render_pipeline_type_mesh_gbuffer有3个描述符集布局）
			1,	// 描述符集数量
			&material.m_materialDescriptorSet,	// 材质 描述符集
			0,
			NULL);

		for (auto& pair2 : meshInstanced)	// 根据网格数据遍历
		{
			ST_VulkanMesh& mesh = (*pair2.first);
			auto& meshNodes = pair2.second;

			uint32_t totalInstanceCount = static_cast<uint32_t>(meshNodes.size());	// 所有网格节点数量
			if (totalInstanceCount > 0)
			{
				// bind per mesh
				m_pRHI->CmdBindDescriptorSetsPFN(
					m_pRHI->GetCurrentCommandBuffer(),
					RHI_PIPELINE_BIND_POINT_GRAPHICS,
					m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipelineLayout,
					1,
					1,
					&mesh.m_meshVertexBlendingDescriptorSet,
					0,
					NULL);

				// 顶点缓存数据
				RHIBuffer* pVertexBuffers[] = {
						mesh.m_meshVertexPositionBuffer,	// 顶点位置
						mesh.m_meshVertexVaryingEnableBlendingBuffer,	// 法线和切线方向
						mesh.m_meshVertexVaryingBuffer	// 纹理坐标
				};

				// 每一块顶点缓冲区的偏移量，从哪开始读取数据
				RHIDeviceSize offsets[] = { 0, 0, 0 };
				// 绑定顶线缓存
				m_pRHI->CmdBindVertexBuffersPFN(m_pRHI->GetCurrentCommandBuffer(), 0, (sizeof(pVertexBuffers) / sizeof(pVertexBuffers[0])), pVertexBuffers, offsets);
				// 绑定索引缓存
				m_pRHI->CmdBindIndexBufferPFN(m_pRHI->GetCurrentCommandBuffer(), mesh.m_meshIndexBuffer, 0, RHI_INDEX_TYPE_UINT16);

				uint32_t drawcallMaxInstanceCount = (sizeof(ST_MeshPerdrawcallStorageBufferObject::m_meshInstances) / sizeof(ST_MeshPerdrawcallStorageBufferObject::m_meshInstances[0]));
				uint32_t drawcallCount = RoundUp(totalInstanceCount, drawcallMaxInstanceCount) / drawcallMaxInstanceCount;

				for (uint32_t drawcallIndex = 0; drawcallIndex < drawcallCount; ++drawcallIndex)
				{
					// 当前要绘制的node数量
					uint32_t currentInstanceCount = ((totalInstanceCount - drawcallMaxInstanceCount * drawcallIndex) < drawcallMaxInstanceCount) ?
						(totalInstanceCount - drawcallMaxInstanceCount * drawcallIndex) :
						drawcallMaxInstanceCount;

					uint32_t perdrawcallDynamicOffset = RoundUp(
						m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
						m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
					);

					m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] = perdrawcallDynamicOffset + sizeof(ST_MeshPerdrawcallStorageBufferObject);

					assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
						(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
							m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

					ST_MeshPerdrawcallStorageBufferObject& perdrawcallStorageBufferObject = (*reinterpret_cast<ST_MeshPerdrawcallStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perdrawcallDynamicOffset));

					for (uint32_t i = 0; i < currentInstanceCount; ++i)
					{
						perdrawcallStorageBufferObject.m_meshInstances[i].m_modelMatrix = *meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_modelMatrix;
						perdrawcallStorageBufferObject.m_meshInstances[i].m_enableVertexBlending = meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_jointMatrices ? 1.0f : -1.0f;
					}

					// 每个drawcall顶点混合存储缓冲区
					uint32_t perDrawcallVertexBlendingDynamicOffset;
					bool leastOneEnableVertexBlending = true;	// 
					for (uint32_t i = 0; i < currentInstanceCount; ++i)
					{
						if (!meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_jointMatrices)
						{
							leastOneEnableVertexBlending = false;
							break;
						}
					}

					if (leastOneEnableVertexBlending)
					{
						perDrawcallVertexBlendingDynamicOffset = RoundUp(
							m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
							m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
						);

						m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] =
							perDrawcallVertexBlendingDynamicOffset + sizeof(ST_MeshPerdrawcallVertexBlendingStorageBufferObject);

						assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
							(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
								m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

						ST_MeshPerdrawcallVertexBlendingStorageBufferObject& perDrawcallVertexBlendingStorageBufferObject = (*reinterpret_cast<ST_MeshPerdrawcallVertexBlendingStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perDrawcallVertexBlendingDynamicOffset));

						for (uint32_t i = 0; i < currentInstanceCount; ++i)
						{
							if (meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_jointMatrices)
							{
								for (uint32_t j = 0; j < meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_jointCount; ++j)
								{
									perDrawcallVertexBlendingStorageBufferObject.m_jointMatrices[s_meshVertexBlendingMaxJointCount * i + j] = meshNodes[drawcallMaxInstanceCount * drawcallIndex + i].m_jointMatrices[j];
								}
							}
						}

					}
					else
					{
						perDrawcallVertexBlendingDynamicOffset = 0;
					}

					uint32_t dynamicOffsets[3] = {
						perframeDynamicOffset,
						perdrawcallDynamicOffset,
						perDrawcallVertexBlendingDynamicOffset
					};

					m_pRHI->CmdBindDescriptorSetsPFN(
						m_pRHI->GetCurrentCommandBuffer(),
						RHI_PIPELINE_BIND_POINT_GRAPHICS,
						m_renderPipelines[_render_pipeline_type_mesh_gbuffer].m_pipelineLayout,
						0,
						1,
						&m_descriptorInfos[_mesh_global].m_pDescriptorSet,
						3,
						dynamicOffsets);

					// 实例化绘制，一次绘制多个实例
					m_pRHI->CmdDrawIndexedPFN(m_pRHI->GetCurrentCommandBuffer(), mesh.m_meshIndexCount, currentInstanceCount, 0, 0, 0);
				}
			}
		}
	}

	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());	// 结束
}

void MainCameraPass::DrawDeferredLighting()
{
	m_pRHI->CmdBindPipelinePFN(m_pRHI->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[_render_pipeline_type_deferred_lighting].m_pipeline);
	m_pRHI->CmdSetViewportPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pViewport);
	m_pRHI->CmdSetScissorPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pScissor);

	uint32_t perframeDynamicOffset = RoundUp(
		m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
		m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
	);

	m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] = perframeDynamicOffset + sizeof(ST_MeshPerframeStorageBufferObject);

	assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
		(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
			m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

	(*reinterpret_cast<ST_MeshPerframeStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perframeDynamicOffset)) = m_meshPerframeStorageBufferObject;

	RHIDescriptorSet* descriptorSets[3] = {
		m_descriptorInfos[_mesh_global].m_pDescriptorSet,
		m_descriptorInfos[_deferred_lighting].m_pDescriptorSet,
		m_descriptorInfos[_skybox].m_pDescriptorSet
	};
	// 4个动态偏移量，_mesh_global有1个，skybox有1个
	uint32_t dynamicOffsets[4] = { perframeDynamicOffset, perframeDynamicOffset, 0, 0 };

	m_pRHI->CmdBindDescriptorSetsPFN(
		m_pRHI->GetCurrentCommandBuffer(),
		RHI_PIPELINE_BIND_POINT_GRAPHICS,
		m_renderPipelines[_render_pipeline_type_deferred_lighting].m_pipelineLayout,
		0,
		3,
		descriptorSets,
		4,
		dynamicOffsets);

	m_pRHI->CmdDraw(m_pRHI->GetCurrentCommandBuffer(), 3, 1, 0, 0);
}

void MainCameraPass::DrawMeshLighting()
{
	// 前向渲染使用
}

void MainCameraPass::DrawSkybox()
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Skybox", color);

	m_pRHI->CmdBindPipelinePFN(m_pRHI->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[_render_pipeline_type_skybox].m_pipeline);

	// 每帧缓冲的动态偏移量
	uint32_t perframeDynamicOffset = RoundUp(
		m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
		m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
	);

	m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] = perframeDynamicOffset + sizeof(ST_MeshPerframeStorageBufferObject);

	assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
		(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
			m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

	(*reinterpret_cast<ST_MeshPerframeStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perframeDynamicOffset)) = m_meshPerframeStorageBufferObject;

	m_pRHI->CmdBindDescriptorSetsPFN(
		m_pRHI->GetCurrentCommandBuffer(),
		RHI_PIPELINE_BIND_POINT_GRAPHICS,
		m_renderPipelines[_render_pipeline_type_skybox].m_pipelineLayout,
		0,
		1,
		&m_descriptorInfos[_skybox].m_pDescriptorSet,
		1,
		&perframeDynamicOffset);

	m_pRHI->CmdDraw(m_pRHI->GetCurrentCommandBuffer(), 36, 1, 0, 0); // 2 triangles(6 vertex) each face, 6 faces

	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());
}

void MainCameraPass::DrawAxis()
{
	if (!m_isShowAxis)
		return;

	// 每帧缓冲的动态偏移量
	uint32_t perframeDynamicOffset = RoundUp(
		m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()],
		m_pGlobalRenderResource->m_storageBuffer.m_minStorageBufferOffsetAlignment
	);

	m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] = perframeDynamicOffset + sizeof(ST_MeshPerframeStorageBufferObject);

	assert(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersEnd[m_pRHI->GetCurrentFrameIndex()] <=
		(m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersBegin[m_pRHI->GetCurrentFrameIndex()] +
			m_pGlobalRenderResource->m_storageBuffer.m_globalUploadRingbuffersSize[m_pRHI->GetCurrentFrameIndex()]));

	(*reinterpret_cast<ST_MeshPerframeStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pGlobalUploadRingbufferMemoryPointer) + perframeDynamicOffset)) = m_meshPerframeStorageBufferObject;

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Axis", color);

	m_pRHI->CmdBindPipelinePFN(m_pRHI->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[_render_pipeline_type_axis].m_pipeline);
	m_pRHI->CmdSetViewportPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pViewport);
	m_pRHI->CmdSetScissorPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pScissor);

	m_axisStorageBufferObject.m_selectedAxis = static_cast<uint32_t>(m_selectedAxis);
	m_axisStorageBufferObject.m_modelMatrix = s_visibleNodes.m_pAxisNode->m_modelMatrix;

	RHIBuffer* vertexBuffers[3] = {
		s_visibleNodes.m_pAxisNode->m_refMesh->m_meshVertexPositionBuffer,
		s_visibleNodes.m_pAxisNode->m_refMesh->m_meshVertexVaryingEnableBlendingBuffer,
		s_visibleNodes.m_pAxisNode->m_refMesh->m_meshVertexVaryingBuffer
	};

	RHIDeviceSize offsets[3] = { 0, 0, 0 };

	m_pRHI->CmdBindVertexBuffersPFN(
		m_pRHI->GetCurrentCommandBuffer(),
		0,
		(sizeof(vertexBuffers) / sizeof(vertexBuffers[0])),
		vertexBuffers,
		offsets);

	m_pRHI->CmdBindIndexBufferPFN(
		m_pRHI->GetCurrentCommandBuffer(),
		s_visibleNodes.m_pAxisNode->m_refMesh->m_meshIndexBuffer,
		0,
		RHI_INDEX_TYPE_UINT16);

	(*reinterpret_cast<ST_AxisStorageBufferObject*>(reinterpret_cast<uintptr_t>(m_pGlobalRenderResource->m_storageBuffer.m_pAxisInefficientStorageBufferMemoryPointer))) = m_axisStorageBufferObject;

	m_pRHI->CmdDrawIndexedPFN(m_pRHI->GetCurrentCommandBuffer(), s_visibleNodes.m_pAxisNode->m_refMesh->m_meshIndexCount, 1, 0, 0, 0);

	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());
}

NAMESPACE_XYH_END

