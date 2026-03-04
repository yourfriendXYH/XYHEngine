#include "ParticlePass.h"
#include <Runtime\Core\Macro.h>
#include <ParticleKickoffComp.h>
#include <ParticleEmitComp.h>
#include <ParticleSimulateComp.h>
#include <ParticleBillboardVert.h>
#include <ParticleBillboardFrag.h>

#include <Runtime/Function/Render/RenderSystem.h>
#include <Runtime/Function/Render/RenderCamera.h>
#include <Runtime/Function/Particle/ParticleCommon.h>

NAMESPACE_XYH_BEGIN

void ParticlePass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);

	const ST_ParticlePassInitInfo* pParticlePassInitInfo = static_cast<const ST_ParticlePassInitInfo*>(initInfo);
	assert(pParticlePassInitInfo != nullptr);
	m_pParticleManager = pParticlePassInitInfo->m_pParticleManager;	// 获取粒子管理器
}

void ParticlePass::PreparePassData(std::shared_ptr<RenderResourceBase> pRenderResource)
{
	// 每帧执行（初始化部分渲染数据）
	const RenderResource* pVulkanResource = static_cast<const RenderResource*>(pRenderResource.get());
	if (pVulkanResource)
	{
		m_particleCollisionPerframeStorageBufferObject = pVulkanResource->m_particleCollisionPerframeStorageBufferObject;
		memcpy(m_pSceneUniformBufferMapped, &m_particleCollisionPerframeStorageBufferObject, sizeof(ST_ParticleCollisionPerframeStorageBufferObject));

		m_particleBillboardPerframeStorageBufferObject = pVulkanResource->m_particleBillboardPerframeStorageBufferObject;
		memcpy(m_pParticleBillboardUniformBufferMapped, &m_particleBillboardPerframeStorageBufferObject, sizeof(ST_ParticleBillboardPerframeStorageBufferObject));

		// 交换链视口数据
		m_viewportParams = *m_pRHI->GetSwapchainInfo().m_pViewport;

		// 更新 m_ubo 数据
		UpdateUniformBuffer();

		// 更新发射器的变换数据
		UpdateEmitterTransform();
	}
}

void ParticlePass::SetRenderCommandBufferHandle(RHICommandBuffer* commandBuffer)
{
	m_pRenderCommandBuffer = commandBuffer;
}

void ParticlePass::SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage)
{
	// MainCameraPass的深度图像和法线图像
	m_pSrcDepthImage = pDepthImage;
	m_pSrcNormalImage = pNormalImage;
}

void ParticlePass::SetupParticlePass()
{
	PrepareUniformBuffer();	// 着色器部分缓冲数据创建

	SetupDescriptorSetLayout();	// 描述符集布局创建

	SetupPipelines();	// 渲染管线创建

	SetupAttachments();	// Image附件创建

	// 命令缓冲创建
	ST_RHICommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.m_pCommandPool = m_pRHI->GetCommandPoor();
	cmdBufAllocateInfo.m_level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.m_commandBufferCount = 1;	// 次级命令缓冲区
	if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, m_pComputeCommandBuffer))
		throw std::runtime_error("alloc compute command buffer");
	if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, m_pCopyCommandBuffer))
		throw std::runtime_error("alloc copy command buffer");

	// 栅栏创建
	ST_RHIFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.m_sType = RHI_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.m_flags = 0;	// 初始状态：未触发
	if (RHI_SUCCESS != m_pRHI->CreateFence(&fenceCreateInfo, m_pFence))
		throw std::runtime_error("create fence");
}

void ParticlePass::CopyNormalAndDepthImage()
{
	// 上一个索引
	uint8_t lastIndex = (m_pRHI->GetCurrentFrameIndex() + m_pRHI->GetMaxFramesInFlight() - 1) % m_pRHI->GetMaxFramesInFlight();

	// 等待上一个帧缓冲的栅栏
	m_pRHI->WaitForFencesPFN(1, &(m_pRHI->GetFenceList()[lastIndex]), VK_TRUE, UINT64_MAX);

	ST_RHICommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.m_flags = 0;
	commandBufferBeginInfo.m_pInheritanceInfo = nullptr;

	bool resBeginCommandBuffer = m_pRHI->BeginCommandBufferPFN(m_pCopyCommandBuffer, &commandBufferBeginInfo);
	assert(RHI_SUCCESS == resBeginCommandBuffer);

	// 复制粒子的深度图像
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pCopyCommandBuffer, "Copy Depth Image for Particle", color);
	// depth image
	ST_RHIImageSubresourceRange subresourceRange = { RHI_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
	ST_RHIImageMemoryBarrier imagememorybarrier{};
	imagememorybarrier.m_sType = RHI_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imagememorybarrier.m_srcQueueFamilyIndex = RHI_QUEUE_FAMILY_IGNORED;
	imagememorybarrier.m_dstQueueFamilyIndex = RHI_QUEUE_FAMILY_IGNORED;
	imagememorybarrier.m_subresourceRange = subresourceRange;
	{
		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imagememorybarrier.m_srcAccessMask = 0;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_pImage = m_pDstDepthImage;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_TRANSFER_READ_BIT;
		imagememorybarrier.m_pImage = m_pSrcDepthImage;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		m_pRHI->CmdCopyImageToImage(
			m_pCopyCommandBuffer,
			m_pSrcDepthImage,
			RHI_IMAGE_ASPECT_DEPTH_BIT,
			m_pDstDepthImage,
			RHI_IMAGE_ASPECT_DEPTH_BIT,
			m_pRHI->GetSwapchainInfo().m_extent.m_width,
			m_pRHI->GetSwapchainInfo().m_extent.m_height);

		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | RHI_ACCESS_SHADER_READ_BIT;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		imagememorybarrier.m_pImage = m_pDstDepthImage;
		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);
	}
	m_pRHI->PopEvent(m_pCopyCommandBuffer); // end depth image copy label

	// 复制粒子的法线图像
	m_pRHI->PushEvent(m_pCopyCommandBuffer, "Copy Normal Image for Particle", color);
	// color image
	subresourceRange = { RHI_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imagememorybarrier.m_subresourceRange = subresourceRange;
	{
		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;	// 传输目标布局
		// 直接传输写入
		imagememorybarrier.m_srcAccessMask = 0;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_pImage = m_pDstNormalImage;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;	// 传输源布局
		// 只有 颜色附件写入 完成，才能 传输读取
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_TRANSFER_READ_BIT;
		imagememorybarrier.m_pImage = m_pSrcNormalImage;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		m_pRHI->CmdCopyImageToImage(
			m_pCopyCommandBuffer,
			m_pSrcNormalImage,
			RHI_IMAGE_ASPECT_COLOR_BIT,
			m_pDstNormalImage,
			RHI_IMAGE_ASPECT_COLOR_BIT,
			m_pRHI->GetSwapchainInfo().m_extent.m_width,
			m_pRHI->GetSwapchainInfo().m_extent.m_height);

		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;	// 传输源布局
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器采样优化布局
		// 只有 传输写入 完成，才能 颜色附件读取 和 着色器读取
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT | RHI_ACCESS_SHADER_READ_BIT;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);

		imagememorybarrier.m_pImage = m_pDstNormalImage;
		imagememorybarrier.m_oldLayout = RHI_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;	// 传输目标布局
		imagememorybarrier.m_newLayout = RHI_IMAGE_LAYOUT_GENERAL;	// 通用布局
		// 只有 传输写入 完成，才能 着色器读取
		imagememorybarrier.m_srcAccessMask = RHI_ACCESS_TRANSFER_WRITE_BIT;
		imagememorybarrier.m_dstAccessMask = RHI_ACCESS_SHADER_READ_BIT;

		m_pRHI->CmdPipelineBarrier(
			m_pCopyCommandBuffer,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imagememorybarrier);
	}
	m_pRHI->PopEvent(m_pCopyCommandBuffer);

	// 结束命令记录
	bool resEndCommandBuffer = m_pRHI->EndCommandBufferPFN(m_pCopyCommandBuffer);
	assert(RHI_SUCCESS == resEndCommandBuffer);

	// 重置上一个帧缓冲的栅栏
	bool resResetFences = m_pRHI->ResetFencesPFN(1, &m_pRHI->GetFenceList()[lastIndex]);
	assert(RHI_SUCCESS == resResetFences);

	RHIPipelineStageFlags waitStages[] = { RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	ST_RHISubmitInfo submitInfo = {};
	submitInfo.m_sType = RHI_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.m_waitSemaphoreCount = 1;
	submitInfo.m_pWaitSemaphores = &(m_pRHI->GetTextureCopySemaphore(lastIndex));
	submitInfo.m_pWaitDstStageMask = waitStages;
	submitInfo.m_commandBufferCount = 1;
	submitInfo.m_pCommandBuffers = &m_pCopyCommandBuffer;
	submitInfo.m_signalSemaphoreCount = 0;
	submitInfo.m_pSignalSemaphores = nullptr;
	bool resQueueSubmit = m_pRHI->QueueSubmit(m_pRHI->GetGraphicsQueue(), 1, &submitInfo, m_pRHI->GetFenceList()[lastIndex]);
	assert(RHI_SUCCESS == resQueueSubmit);

	m_pRHI->QueueWaitIdle(m_pRHI->GetGraphicsQueue());
}

void ParticlePass::Simulate()
{
	// 执行计算着色器
	for (auto i : m_emitterTickIndices)
	{

	}
	m_emitterTickIndices.clear();
	m_emitterTransformIndices.clear();
}

void ParticlePass::SetRenderPassHandle(RHIRenderPass* pRenderPass)
{
	m_pRenderPass = pRenderPass;
}

void ParticlePass::UpdateAfterFramebufferRecreate()
{
	// 重置深度图像
	m_pRHI->DestroyImage(m_pDstDepthImage);
	m_pRHI->FreeMemory(m_pDstDepthImageMemory);
	m_pRHI->CreateImage(
		m_pRHI->GetSwapchainInfo().m_extent.m_width,
		m_pRHI->GetSwapchainInfo().m_extent.m_height,
		m_pRHI->GetDepthImageInfo().m_depthImageFormat,
		RHI_IMAGE_TILING_OPTIMAL,
		RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT,
		RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_pDstDepthImage,
		m_pDstDepthImageMemory,
		0,
		1,
		1);

	// 重置法线图像
	m_pRHI->DestroyImage(m_pDstNormalImage);
	m_pRHI->FreeMemory(m_pDstNormalImageMemory);
	m_pRHI->CreateImage(
		m_pRHI->GetSwapchainInfo().m_extent.m_width,
		m_pRHI->GetSwapchainInfo().m_extent.m_height,
		RHI_FORMAT_R8G8B8A8_UNORM,
		RHI_IMAGE_TILING_OPTIMAL,
		RHI_IMAGE_USAGE_STORAGE_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT,
		RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_pDstNormalImage,
		m_pDstNormalImageMemory,
		0,
		1,
		1);

	m_pRHI->CreateImageView(
		m_pDstDepthImage,
		m_pRHI->GetDepthImageInfo().m_depthImageFormat,
		RHI_IMAGE_ASPECT_DEPTH_BIT,
		RHI_IMAGE_VIEW_TYPE_2D,
		1,
		1,
		m_pSrcDepthImageView);

	m_pRHI->CreateImageView(
		m_pDstNormalImage,
		RHI_FORMAT_R8G8B8A8_UNORM,
		RHI_IMAGE_ASPECT_COLOR_BIT,
		RHI_IMAGE_VIEW_TYPE_2D,
		1,
		1,
		m_pSrcNormalImageView);

	// 更新粒子发射器的描述符集（0、1）
	UpdateDescriptorSet();
}

void ParticlePass::SetEmitterCount(int count)
{
	for (size_t i = 0; i < m_emitterBufferBatches.size(); ++i)
	{
		m_emitterBufferBatches[i].FreeUpBatch(m_pRHI);
	}
	m_emitterCount = count;
	m_emitterBufferBatches.resize(m_emitterCount);
}

void ParticlePass::CreateEmitter(int id, const ST_ParticleEmitterDesc& desc)
{
	const VkDeviceSize counterBufferSize = sizeof(ST_ParticleCounter);
	ST_ParticleCounter counter;
	counter.m_aliveCount = m_emitterBufferBatches[id].m_numParticle;
	counter.m_deadCount = s_maxParticles - m_emitterBufferBatches[id].m_numParticle;
	counter.m_emitCount = 0;
	counter.m_aliveCountAfterSim = m_emitterBufferBatches[id].m_numParticle;

	if constexpr (s_verboseParticleAliveInfo)
	{
		LOG_INFO("Emitter {} info:", id);
		LOG_INFO("Dead {}, Alive {}, After sim {}, Emit {}",
			counter.m_deadCount,
			counter.m_aliveCount,
			counter.m_aliveCountAfterSim,
			counter.m_emitCount);
	}

	{
		// ???
		const VkDeviceSize indirectArgumentSize = sizeof(ST_IndirectArgumemt);
		struct ST_IndirectArgumemt indirectArgument = {};
		indirectArgument.m_aliveFlapBit = 1;
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_emitterBufferBatches[id].m_pIndirectDispatchArgumentBuffer,
			m_emitterBufferBatches[id].m_pIndirectDispatchArgumentMemory,
			indirectArgumentSize,
			&indirectArgument,
			indirectArgumentSize);

		// ???
		const VkDeviceSize aliveListSize = 4 * sizeof(uint32_t) * s_maxParticles;
		std::vector<int> aliveindices(s_maxParticles * 4, 0);
		for (int i = 0; i < s_maxParticles; ++i)
			aliveindices[i * 4] = i;
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_emitterBufferBatches[id].m_pAliveListBuffer,
			m_emitterBufferBatches[id].m_pAliveListMemory,
			aliveListSize,
			aliveindices.data(),
			aliveListSize);
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_emitterBufferBatches[id].m_pAliveListNextBuffer,
			m_emitterBufferBatches[id].m_pAliveListNextMemory,
			aliveListSize);

		// ???
		const VkDeviceSize deadListSize = 4 * sizeof(uint32_t) * s_maxParticles;
		std::vector<int32_t> deadindices(s_maxParticles * 4, 0);
		for (int32_t i = 0; i < s_maxParticles; ++i)
			deadindices[i * 4] = s_maxParticles - 1 - i;
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_emitterBufferBatches[id].m_pDeadListBuffer,
			m_emitterBufferBatches[id].m_pDeadListMemory,
			deadListSize,
			deadindices.data(),
			deadListSize);
	}

	RHIFence* fence = nullptr;
	ST_ParticleCounter counterNext{};
	{
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_emitterBufferBatches[id].m_pCounterHostBuffer,
			m_emitterBufferBatches[id].m_pCounterHostMemory,
			counterBufferSize,
			&counter,
			sizeof(counter));
		// 将刷新写入主要可见缓冲区
		void* mapped;
		m_pRHI->MapMemory(m_emitterBufferBatches[id].m_pCounterHostMemory, 0, RHI_WHOLE_SIZE, 0, &mapped);
		m_pRHI->FlushMappedMemoryRanges(nullptr, m_emitterBufferBatches[id].m_pCounterHostMemory, 0, RHI_WHOLE_SIZE);
		m_pRHI->UnmapMemory(m_emitterBufferBatches[id].m_pCounterHostMemory);

		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_SRC_BIT |
			RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
			RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_emitterBufferBatches[id].m_pCounterDeviceBuffer,
			m_emitterBufferBatches[id].m_pCounterDeviceMemory,
			counterBufferSize);

		// Copy to staging buffer
		ST_RHICommandBufferAllocateInfo cmdBufAllocateInfo{};
		cmdBufAllocateInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.m_pCommandPool = m_pRHI->GetCommandPoor();
		cmdBufAllocateInfo.m_level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.m_commandBufferCount = 1;
		RHICommandBuffer* pCopyCmd;
		if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, pCopyCmd))
		{
			throw std::runtime_error("alloc command buffer");
		}
		ST_RHICommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (RHI_SUCCESS != m_pRHI->BeginCommandBuffer(pCopyCmd, &cmdBufInfo))
		{
			throw std::runtime_error("begin command buffer");
		}

		ST_RHIBufferCopy copyRegion = {};
		copyRegion.m_srcOffset = 0;
		copyRegion.m_dstOffset = 0;
		copyRegion.m_size = counterBufferSize;
		m_pRHI->CmdCopyBuffer(
			pCopyCmd,
			m_emitterBufferBatches[id].m_pCounterHostBuffer,
			m_emitterBufferBatches[id].m_pCounterDeviceBuffer,
			1,
			&copyRegion);

		if (RHI_SUCCESS != m_pRHI->EndCommandBuffer(pCopyCmd))
		{
			throw std::runtime_error("buffer copy");
		}

		// 提交拷贝命令
		ST_RHISubmitInfo submitInfo{};
		submitInfo.m_sType = RHI_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.m_commandBufferCount = 1;
		submitInfo.m_pCommandBuffers = &pCopyCmd;
		ST_RHIFenceCreateInfo fenceInfo{};
		fenceInfo.m_sType = RHI_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.m_flags = 0;
		if (RHI_SUCCESS != m_pRHI->CreateFence(&fenceInfo, fence))
		{
			throw std::runtime_error("create fence");
		}
		// Submit to the queue
		if (RHI_SUCCESS != m_pRHI->QueueSubmit(m_pRHI->GetComputeQueue(), 1, &submitInfo, fence))
		{
			throw std::runtime_error("queue submit");
		}

		if (RHI_SUCCESS != m_pRHI->WaitForFencesPFN(1, &fence, RHI_TRUE, UINT64_MAX))
		{
			throw std::runtime_error("wait fence submit");
		}
		m_pRHI->DestroyFence(fence);
		m_pRHI->FreeCommandBuffers(m_pRHI->GetCommandPoor(), 1, pCopyCmd);
	}

	const VkDeviceSize staggingBuferSize = s_maxParticles * sizeof(ST_Particle);
	m_emitterBufferBatches[id].m_emitterDesc = desc;
	{
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_emitterBufferBatches[id].m_pParticleComponentResBuffer,
			m_emitterBufferBatches[id].m_pParticleComponentResMemory,
			sizeof(ST_ParticleEmitterDesc),
			&m_emitterBufferBatches[id].m_emitterDesc,
			sizeof(ST_ParticleEmitterDesc));

		if (RHI_SUCCESS != m_pRHI->MapMemory(
			m_emitterBufferBatches[id].m_pParticleComponentResMemory,
			0,
			RHI_WHOLE_SIZE,
			0,
			&m_emitterBufferBatches[id].m_pEmitterDescMapped))
		{
			throw std::runtime_error("map emitter component res buffer");
		}

		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_emitterBufferBatches[id].m_pPositionHostBuffer,
			m_emitterBufferBatches[id].m_pPositionHostMemory,
			staggingBuferSize);
		// Flush writes to host visible buffer
		void* mapped;
		m_pRHI->MapMemory(m_emitterBufferBatches[id].m_pPositionHostMemory, 0, RHI_WHOLE_SIZE, 0, &mapped);
		m_pRHI->FlushMappedMemoryRanges(nullptr, m_emitterBufferBatches[id].m_pPositionHostMemory, 0, RHI_WHOLE_SIZE);
		m_pRHI->UnmapMemory(m_emitterBufferBatches[id].m_pPositionHostMemory);

		//
		m_pRHI->CreateBufferAndInitialize(
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_SRC_BIT |
			RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
			RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_emitterBufferBatches[id].m_pPositionDeviceBuffer,
			m_emitterBufferBatches[id].m_pPositionDeviceMemory,
			staggingBuferSize);

		m_pRHI->CreateBufferAndInitialize(RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_SRC_BIT |
			RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
			RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_emitterBufferBatches[id].m_pPositionRenderBuffer,
			m_emitterBufferBatches[id].m_pPositionRenderMemory,
			staggingBuferSize);

		// Copy to staging buffer
		ST_RHICommandBufferAllocateInfo cmdBufAllocateInfo{};
		cmdBufAllocateInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.m_pCommandPool = m_pRHI->GetCommandPoor();
		cmdBufAllocateInfo.m_level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.m_commandBufferCount = 1;
		RHICommandBuffer* pCopyCmd;
		if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, pCopyCmd))
		{
			throw std::runtime_error("alloc command buffer");
		}
		ST_RHICommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (RHI_SUCCESS != m_pRHI->BeginCommandBuffer(pCopyCmd, &cmdBufInfo))
		{
			throw std::runtime_error("begin command buffer");
		}

		ST_RHIBufferCopy copyRegion = {};
		copyRegion.m_srcOffset = 0;
		copyRegion.m_dstOffset = 0;
		copyRegion.m_size = staggingBuferSize;
		m_pRHI->CmdCopyBuffer(
			pCopyCmd,
			m_emitterBufferBatches[id].m_pPositionHostBuffer,
			m_emitterBufferBatches[id].m_pPositionDeviceBuffer,
			1,
			&copyRegion);

		if (RHI_SUCCESS != m_pRHI->EndCommandBuffer(pCopyCmd))
		{
			throw std::runtime_error("buffer copy");
		}

		ST_RHISubmitInfo submitInfo{};
		submitInfo.m_sType = RHI_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.m_commandBufferCount = 1;
		submitInfo.m_pCommandBuffers = &pCopyCmd;
		ST_RHIFenceCreateInfo fenceInfo{};
		fenceInfo.m_sType = RHI_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.m_flags = 0;
		if (RHI_SUCCESS != m_pRHI->CreateFence(&fenceInfo, fence))
		{
			throw std::runtime_error("create fence");
		}

		// Submit to the queue
		if (RHI_SUCCESS != m_pRHI->QueueSubmit(m_pRHI->GetComputeQueue(), 1, &submitInfo, fence))
		{
			throw std::runtime_error("queue submit");
		}

		if (RHI_SUCCESS != m_pRHI->WaitForFencesPFN(1, &fence, RHI_TRUE, UINT64_MAX))
		{
			throw std::runtime_error("wait fence submit");
		}

		m_pRHI->DestroyFence(fence);
		m_pRHI->FreeCommandBuffers(m_pRHI->GetCommandPoor(), 1, pCopyCmd);
	}
}

void ParticlePass::InitializeEmitters()
{
	AllocateDescriptorSet();	// 分配发射器描述符集内存

	UpdateDescriptorSet();	// 更新粒子发射器的描述符集（0、1）

	SetupParticleDescriptorSet();	// 设置第三个描述符集（2）
}

void ParticlePass::SetTickIndices(const std::vector<ParticleEmitterID>& tickIndices)
{
	m_emitterTickIndices = tickIndices;
}

void ParticlePass::SetTransformIndices(const std::vector<ST_ParticleEmitterTransformDesc>& transformIndices)
{
	m_emitterTransformIndices = transformIndices;
}

void ParticlePass::Draw()
{
	for (int i = 0; i < m_emitterCount; ++i)
	{
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_pRHI->PushEvent(m_pRenderCommandBuffer, "ParticleBillboard", color);

		// 绑定粒子的图形管线，索引0是计算管线
		m_pRHI->CmdBindPipelinePFN(m_pRenderCommandBuffer, RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[1].m_pipeline);
		m_pRHI->CmdSetViewportPFN(m_pRenderCommandBuffer, 0, 1, m_pRHI->GetSwapchainInfo().m_pViewport);
		m_pRHI->CmdSetScissorPFN(m_pRenderCommandBuffer, 0, 1, m_pRHI->GetSwapchainInfo().m_pScissor);

		// 绑定描述符集
		m_pRHI->CmdBindDescriptorSetsPFN(
			m_pRenderCommandBuffer,
			RHI_PIPELINE_BIND_POINT_GRAPHICS,
			m_renderPipelines[1].m_pipelineLayout,
			0,
			1,
			&m_descriptorInfos[i * 3 + 2].m_pDescriptorSet,
			0,
			nullptr
		);

		// 绘制
		m_pRHI->CmdDraw(m_pRenderCommandBuffer, 4, m_emitterBufferBatches[i].m_numParticle, 0, 0);

		m_pRHI->PopEvent(m_pRenderCommandBuffer);
	}
}

void ParticlePass::PrepareUniformBuffer()
{
	// 分配 矩阵数据 的内存
	RHIDeviceMemory* pDeviceMemory;
	m_pRHI->CreateBuffer(
		sizeof(m_particleCollisionPerframeStorageBufferObject), // 内存大小
		RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_pSceneUniformBuffer,
		pDeviceMemory);

	if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceMemory, 0, RHI_WHOLE_SIZE, 0, &m_pSceneUniformBufferMapped))    // 映射从偏移量到内存末尾的整个区域
	{
		throw std::runtime_error("map billboard uniform buffer");
	}

	// 分配内存 粒子生成的参数
	RHIDeviceMemory* pDeviceUniformMemory;
	m_pRHI->CreateBufferAndInitialize(
		RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_pComputeUniformBuffer,
		pDeviceUniformMemory,
		sizeof(m_ubo)); // 内存大小
	if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceUniformMemory, 0, RHI_WHOLE_SIZE, 0, &m_pParticleComputeBufferMapped))  // 映射从偏移量到内存末尾的整个区域
	{
		throw std::runtime_error("map buffer");
	}

	// 粒子生成的属性
	const GlobalParticleResource& globalRes = m_pParticleManager->GetGlobalParticleRes();
	m_ubo.m_emitGap = globalRes.m_emitGap;
	m_ubo.m_timeStep = globalRes.m_timeStep;
	m_ubo.m_maxLife = globalRes.m_maxLife;
	m_ubo.m_gravity = globalRes.m_gravity;
	std::random_device r;
	std::seed_seq seed{ r() };
	//m_random_engine.seed(seed);
	//float rnd0 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//float rnd1 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//float rnd2 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//m_ubo.m_pack = Vector4{ rnd0, static_cast<float>(m_pRHI->GetCurrentFrameIndex()), rnd1, rnd2 };
	m_ubo.m_xemitCount = 100000;

	m_viewportParams = *m_pRHI->GetSwapchainInfo().m_pViewport;
	m_ubo.m_viewport.x = static_cast<uint32_t>(m_viewportParams.m_x);
	m_ubo.m_viewport.y = static_cast<uint32_t>(m_viewportParams.m_y);
	m_ubo.m_viewport.z = static_cast<uint32_t>(m_viewportParams.m_width);
	m_ubo.m_viewport.w = static_cast<uint32_t>(m_viewportParams.m_height);
	m_ubo.m_extent.x = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_width);
	m_ubo.m_extent.y = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_height);
	// 给内存赋值
	memcpy(m_pParticleComputeBufferMapped, &m_ubo, sizeof(m_ubo));

	{
		RHIDeviceMemory* pDeviceMemory;
		m_pRHI->CreateBuffer(
			sizeof(m_particleBillboardPerframeStorageBufferObject),
			RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_pParticleBillboardUniformBuffer,
			pDeviceMemory);

		if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceMemory, 0, RHI_WHOLE_SIZE, 0, &m_pParticleBillboardUniformBufferMapped))    // 映射从偏移量到内存末尾的整个区域
		{
			throw std::runtime_error("map billboard uniform buffer");
		}
	}
}

void ParticlePass::SetupAttachments()
{
	// billboard texture
	{
		std::shared_ptr<TextureData> m_pParticleBillboardTextureResource = m_pRenderResource->LoadTextureHDR(m_pParticleManager->GetGlobalParticleRes().m_particleBillboardTexturePath);
		if (nullptr != m_pParticleBillboardTextureResource)
		{
			m_pRHI->CreateGlobalImage(
				m_pParticleBillboardTextureImage,
				m_pParticleBillboardTextureImageView,
				m_particleBillboardTextureVmaAllocation,
				m_pParticleBillboardTextureResource->m_width,
				m_pParticleBillboardTextureResource->m_height,
				m_pParticleBillboardTextureResource->m_pixels,
				m_pParticleBillboardTextureResource->m_format);
		}
	}

	// piccolo texture
	{
		std::shared_ptr<TextureData> m_pPiccoloLogoTextureResource = m_pRenderResource->LoadTexture(m_pParticleManager->GetGlobalParticleRes().m_piccoloLogoTexturePath, true);
		if (nullptr != m_pPiccoloLogoTextureResource)
		{
			m_pRHI->CreateGlobalImage(
				m_pPiccoloLogoTextureImage,
				m_pPiccoloLogoTextureImageView,
				m_piccoloLogoTextureVmaAllocation,
				m_pPiccoloLogoTextureResource->m_width,
				m_pPiccoloLogoTextureResource->m_height,
				m_pPiccoloLogoTextureResource->m_pixels,
				m_pPiccoloLogoTextureResource->m_format);
		}
	}

	// 深度
	m_pRHI->CreateImage(
		m_pRHI->GetSwapchainInfo().m_extent.m_width,
		m_pRHI->GetSwapchainInfo().m_extent.m_height,
		m_pRHI->GetDepthImageInfo().m_depthImageFormat,
		RHI_IMAGE_TILING_OPTIMAL,
		RHI_IMAGE_USAGE_SAMPLED_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT,
		RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_pDstDepthImage,
		m_pDstDepthImageMemory,
		0,
		1,
		1);

	// 法线
	m_pRHI->CreateImage(
		m_pRHI->GetSwapchainInfo().m_extent.m_width,
		m_pRHI->GetSwapchainInfo().m_extent.m_height,
		RHI_FORMAT_R8G8B8A8_UNORM,
		RHI_IMAGE_TILING_OPTIMAL,
		RHI_IMAGE_USAGE_STORAGE_BIT | RHI_IMAGE_USAGE_TRANSFER_DST_BIT,
		RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_pDstNormalImage,
		m_pDstNormalImageMemory,
		0,
		1,
		1);

	m_pRHI->CreateImageView(
		m_pDstDepthImage,
		m_pRHI->GetDepthImageInfo().m_depthImageFormat,
		RHI_IMAGE_ASPECT_DEPTH_BIT,
		RHI_IMAGE_VIEW_TYPE_2D,
		1,
		1,
		m_pSrcDepthImageView);

	m_pRHI->CreateImageView(
		m_pDstNormalImage,
		RHI_FORMAT_R8G8B8A8_UNORM,
		RHI_IMAGE_ASPECT_COLOR_BIT,
		RHI_IMAGE_VIEW_TYPE_2D,
		1,
		1,
		m_pSrcNormalImageView);
}

void ParticlePass::SetupDescriptorSetLayout()
{
	m_descriptorInfos.resize(3u);

	// 
	{
		ST_RHIDescriptorSetLayoutBinding particleLayoutBindings[11] = {};
		{
			ST_RHIDescriptorSetLayoutBinding& uniformLayoutBinding = particleLayoutBindings[0];
			uniformLayoutBinding.m_binding = 0;
			uniformLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformLayoutBinding.m_descriptorCount = 1;
			uniformLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& storagePositionLayoutBinding = particleLayoutBindings[1];
			storagePositionLayoutBinding.m_binding = 1;
			storagePositionLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storagePositionLayoutBinding.m_descriptorCount = 1;
			storagePositionLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& storageCounterLayoutBinding = particleLayoutBindings[2];
			storageCounterLayoutBinding.m_binding = 2;
			storageCounterLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageCounterLayoutBinding.m_descriptorCount = 1;
			storageCounterLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& storageIndirectargumentLayoutBinding = particleLayoutBindings[3];
			storageIndirectargumentLayoutBinding.m_binding = 3;
			storageIndirectargumentLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageIndirectargumentLayoutBinding.m_descriptorCount = 1;
			storageIndirectargumentLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& aliveListLayoutBinding = particleLayoutBindings[4];
			aliveListLayoutBinding.m_binding = 4;
			aliveListLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			aliveListLayoutBinding.m_descriptorCount = 1;
			aliveListLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& deadListLayoutBinding = particleLayoutBindings[5];
			deadListLayoutBinding.m_binding = 5;
			deadListLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			deadListLayoutBinding.m_descriptorCount = 1;
			deadListLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& aliveListNextLayoutBinding = particleLayoutBindings[6];
			aliveListNextLayoutBinding.m_binding = 6;
			aliveListNextLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			aliveListNextLayoutBinding.m_descriptorCount = 1;
			aliveListNextLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& particleResLayoutBinding = particleLayoutBindings[7];
			particleResLayoutBinding.m_binding = 7;
			particleResLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			particleResLayoutBinding.m_descriptorCount = 1;
			particleResLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& sceneUniformbufferLayoutBinding = particleLayoutBindings[8];
			sceneUniformbufferLayoutBinding.m_binding = 8;
			sceneUniformbufferLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			sceneUniformbufferLayoutBinding.m_descriptorCount = 1;
			sceneUniformbufferLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& storageRenderPositionLayoutBinding = particleLayoutBindings[9];
			storageRenderPositionLayoutBinding.m_binding = 9;
			storageRenderPositionLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageRenderPositionLayoutBinding.m_descriptorCount = 1;
			storageRenderPositionLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		{
			ST_RHIDescriptorSetLayoutBinding& xyhTextureLayoutBinding = particleLayoutBindings[10];
			xyhTextureLayoutBinding.m_binding = 10;
			xyhTextureLayoutBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			xyhTextureLayoutBinding.m_descriptorCount = 1;
			xyhTextureLayoutBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;
		}

		ST_RHIDescriptorSetLayoutCreateInfo particleDescriptorLayoutCreateInfo;
		particleDescriptorLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		particleDescriptorLayoutCreateInfo.m_pNext = NULL;
		particleDescriptorLayoutCreateInfo.m_flags = 0;
		particleDescriptorLayoutCreateInfo.m_bindingCount = sizeof(particleLayoutBindings) / sizeof(particleLayoutBindings[0]);
		particleDescriptorLayoutCreateInfo.m_pBindings = particleLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&particleDescriptorLayoutCreateInfo, m_descriptorInfos[0].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("setup particle compute Descriptor done");
		}
		LOG_INFO("setup particle compute Descriptor done");
	}

	// scene depth and normal binding
	{
		ST_RHIDescriptorSetLayoutBinding sceneGlobalLayoutBindings[2] = {};

		ST_RHIDescriptorSetLayoutBinding& gbufferNormalGlobalLayoutInputAttachmentBinding = sceneGlobalLayoutBindings[0];	// 
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_binding = 0;
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferNormalGlobalLayoutInputAttachmentBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;

		ST_RHIDescriptorSetLayoutBinding& gbufferDepthGlobalLayoutInputAttachmentBinding = sceneGlobalLayoutBindings[1];
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_binding = 1;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_descriptorCount = 1;
		gbufferDepthGlobalLayoutInputAttachmentBinding.m_stageFlags = RHI_SHADER_STAGE_COMPUTE_BIT;

		ST_RHIDescriptorSetLayoutCreateInfo gbufferLightingGlobalLayoutCreateInfo;
		gbufferLightingGlobalLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		gbufferLightingGlobalLayoutCreateInfo.m_pNext = NULL;
		gbufferLightingGlobalLayoutCreateInfo.m_flags = 0;
		gbufferLightingGlobalLayoutCreateInfo.m_bindingCount = sizeof(sceneGlobalLayoutBindings) / sizeof(sceneGlobalLayoutBindings[0]);
		gbufferLightingGlobalLayoutCreateInfo.m_pBindings = sceneGlobalLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&gbufferLightingGlobalLayoutCreateInfo, m_descriptorInfos[1].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create scene normal and depth global layout");
		}
	}

	{
		ST_RHIDescriptorSetLayoutBinding particleBillboardGlobalLayoutBindings[3];

		ST_RHIDescriptorSetLayoutBinding& particleBillboardGlobalLayoutPerframeStorageBufferBinding = particleBillboardGlobalLayoutBindings[0];
		particleBillboardGlobalLayoutPerframeStorageBufferBinding.m_binding = 0;
		particleBillboardGlobalLayoutPerframeStorageBufferBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		particleBillboardGlobalLayoutPerframeStorageBufferBinding.m_descriptorCount = 1;
		particleBillboardGlobalLayoutPerframeStorageBufferBinding.m_stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
		particleBillboardGlobalLayoutPerframeStorageBufferBinding.m_pImmutableSamplers = NULL;

		ST_RHIDescriptorSetLayoutBinding& particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding = particleBillboardGlobalLayoutBindings[1];
		particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding.m_binding = 1;
		particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding.m_descriptorCount = 1;
		particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding.m_stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
		particleBillboardGlobalLayoutPerdrawcallStorageBufferBinding.m_pImmutableSamplers = NULL;

		ST_RHIDescriptorSetLayoutBinding& particleBillboardGlobalLayoutTextureBinding = particleBillboardGlobalLayoutBindings[2];
		particleBillboardGlobalLayoutTextureBinding.m_binding = 2;
		particleBillboardGlobalLayoutTextureBinding.m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		particleBillboardGlobalLayoutTextureBinding.m_descriptorCount = 1;
		particleBillboardGlobalLayoutTextureBinding.m_stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
		particleBillboardGlobalLayoutTextureBinding.m_pImmutableSamplers = NULL;

		ST_RHIDescriptorSetLayoutCreateInfo particleBillboardGlobalLayoutCreateInfo;
		particleBillboardGlobalLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		particleBillboardGlobalLayoutCreateInfo.m_pNext = NULL;
		particleBillboardGlobalLayoutCreateInfo.m_flags = 0;
		particleBillboardGlobalLayoutCreateInfo.m_bindingCount = 3;
		particleBillboardGlobalLayoutCreateInfo.m_pBindings = particleBillboardGlobalLayoutBindings;

		if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&particleBillboardGlobalLayoutCreateInfo, m_descriptorInfos[2].m_pDescriptorSetLayout))
		{
			throw std::runtime_error("create particle billboard global layout");
		}
	}
}

void ParticlePass::SetupPipelines()
{
	m_renderPipelines.resize(2u);	// 一个计算管线，一个绘制管线

	// compute pipeline
	{
		// 计算管线布局
		RHIDescriptorSetLayout* descriptorset_layouts[2] = { m_descriptorInfos[0].m_pDescriptorSetLayout, m_descriptorInfos[1].m_pDescriptorSetLayout };
		ST_RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.m_setLayoutCount = sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]);
		pipeline_layout_create_info.m_pSetLayouts = descriptorset_layouts;

		if (m_pRHI->CreatePipelineLayout(&pipeline_layout_create_info, m_renderPipelines[0].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create compute pass pipe layout");
		}
		LOG_INFO("compute pipe layout done");
	}

	// specializationInfo暂时没有使用
	struct ST_SpecializationData
	{
		uint32_t BUFFER_ELEMENT_COUNT = 32;
	} specializationData;

	VkSpecializationMapEntry specializationMapEntry{};
	specializationMapEntry.constantID = 0;
	specializationMapEntry.offset = 0;
	specializationMapEntry.size = sizeof(uint32_t);

	VkSpecializationInfo specializationInfo{};
	specializationInfo.mapEntryCount = 1;
	specializationInfo.pMapEntries = &specializationMapEntry;
	specializationInfo.dataSize = sizeof(specializationData);
	specializationInfo.pData = &specializationData;

	ST_RHIComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.m_sType = RHI_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.m_pLayout = m_renderPipelines[0].m_pipelineLayout;
	computePipelineCreateInfo.m_flags = 0;

	ST_RHIPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.m_stage = RHI_SHADER_STAGE_COMPUTE_BIT;	// 计算着色器
	shaderStage.m_pName = "main";

	// 开始
	{
		shaderStage.m_module = m_pRHI->CreateShaderModule(PARTICLE_KICKOFF_COMP);
		shaderStage.m_pSpecializationInfo = nullptr;
		assert(shaderStage.m_module != RHI_NULL_HANDLE);

		computePipelineCreateInfo.m_pStages = &shaderStage;
		// 创建计算管线
		if (RHI_SUCCESS != m_pRHI->CreateComputePipelines(/*pipelineCache*/ nullptr, 1, &computePipelineCreateInfo, m_pKickoffPipeline))
		{
			throw std::runtime_error("create particle kickoff pipe");
		}
	}

	// 发出
	{
		shaderStage.m_module = m_pRHI->CreateShaderModule(PARTICLE_EMIT_COMP);
		shaderStage.m_pSpecializationInfo = nullptr;
		assert(shaderStage.m_module != RHI_NULL_HANDLE);

		computePipelineCreateInfo.m_pStages = &shaderStage;
		if (RHI_SUCCESS != m_pRHI->CreateComputePipelines(/*pipelineCache*/ nullptr, 1, &computePipelineCreateInfo, m_pEmitPipeline))
		{
			throw std::runtime_error("create particle emit pipe");
		}
	}

	// 模拟
	{
		shaderStage.m_module = m_pRHI->CreateShaderModule(PARTICLE_SIMULATE_COMP);
		shaderStage.m_pSpecializationInfo = nullptr;
		assert(shaderStage.m_module != RHI_NULL_HANDLE);

		computePipelineCreateInfo.m_pStages = &shaderStage;
		if (RHI_SUCCESS != m_pRHI->CreateComputePipelines(/*pipelineCache*/ nullptr, 1, &computePipelineCreateInfo, m_pSimulatePipeline))
		{
			throw std::runtime_error("create particle simulate pipe");
		}
	}

	// 粒子广告牌 是一种渲染技术，它确保一个通常代表粒子的二维四边形（即“广告牌”）始终面向摄像机。
	// 你可以把它想象成在3D世界里放置一张总是正对着你的纸片或图片，无论你如何移动视角。
	{
		// 描述符集
		RHIDescriptorSetLayout* descriptorsetLayouts[1] = { m_descriptorInfos[2].m_pDescriptorSetLayout };
		ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.m_setLayoutCount = 1;
		pipelineLayoutCreateInfo.m_pSetLayouts = descriptorsetLayouts;

		if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[1].m_pipelineLayout) != RHI_SUCCESS)
		{
			throw std::runtime_error("create particle billboard pipeline layout");
		}

		// 着色器阶段
		RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(PARTICLEBILLBOARD_VERT);
		RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(PARTICLEBILLBOARD_FRAG);

		ST_RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
		vertPipelineShaderStageCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertPipelineShaderStageCreateInfo.m_stage = RHI_SHADER_STAGE_VERTEX_BIT;	// 顶点着色器
		vertPipelineShaderStageCreateInfo.m_module = pVertShaderModule;
		vertPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
		fragPipelineShaderStageCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragPipelineShaderStageCreateInfo.m_stage = RHI_SHADER_STAGE_FRAGMENT_BIT;	// 片段着色器
		fragPipelineShaderStageCreateInfo.m_module = pFragShaderModule;
		fragPipelineShaderStageCreateInfo.m_pName = "main";

		ST_RHIPipelineShaderStageCreateInfo shaderStages[] = {
			vertPipelineShaderStageCreateInfo,
			fragPipelineShaderStageCreateInfo
		};

		// 顶点布局
		ST_RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCreateInfo.m_vertexBindingDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexBindingDescriptions = NULL;
		vertexInputStateCreateInfo.m_vertexAttributeDescriptionCount = 0;
		vertexInputStateCreateInfo.m_pVertexAttributeDescriptions = NULL;

		// 输入装配
		ST_RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.m_topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;	// 拓扑
		inputAssemblyCreateInfo.m_primitiveRestartEnable = RHI_FALSE;

		// 视口信息
		ST_RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.m_viewportCount = 1;
		viewportStateCreateInfo.m_pViewports = m_pRHI->GetSwapchainInfo().m_pViewport;
		viewportStateCreateInfo.m_scissorCount = 1;
		viewportStateCreateInfo.m_pScissors = m_pRHI->GetSwapchainInfo().m_pScissor;

		// 光栅化信息
		ST_RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.m_depthClampEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_rasterizerDiscardEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_polygonMode = RHI_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.m_lineWidth = 1.0f;
		rasterizationStateCreateInfo.m_cullMode = RHI_CULL_MODE_NONE;
		rasterizationStateCreateInfo.m_frontFace = RHI_FRONT_FACE_CLOCKWISE;
		rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;
		rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

		// 采样信息
		ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
		multisampleStateCreateInfo.m_rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

		// 颜色混合信息
		ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
		colorBlendAttachments[0].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].m_blendEnable = RHI_TRUE;
		colorBlendAttachments[0].m_srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstColorBlendFactor = RHI_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachments[0].m_colorBlendOp = RHI_BLEND_OP_ADD;
		colorBlendAttachments[0].m_srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
		colorBlendAttachments[0].m_dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
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

		// 深度模板
		ST_RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.m_depthTestEnable = RHI_TRUE;
		depthStencilCreateInfo.m_depthWriteEnable = RHI_FALSE;
		depthStencilCreateInfo.m_depthCompareOp = RHI_COMPARE_OP_LESS;
		depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
		depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

		// 动态数据（可在渲染执行时修改）
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
		pipelineInfo.m_pLayout = m_renderPipelines[1].m_pipelineLayout;
		pipelineInfo.m_pRenderPass = m_pRenderPass;
		pipelineInfo.m_subpass = _main_camera_subpass_forward_lighting;
		pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
		pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

		// 创建用于粒子显示的管线
		if (m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[1].m_pipeline) !=
			RHI_SUCCESS)
		{
			throw std::runtime_error("create particle billboard graphics pipeline");
		}

		m_pRHI->DestroyShaderModule(pVertShaderModule);
		m_pRHI->DestroyShaderModule(pFragShaderModule);
	}
}

void ParticlePass::UpdateUniformBuffer()
{
	std::random_device r;
	std::seed_seq seed{ r() };
	//m_randomEngine.seed(seed);
	//float rnd0 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//float rnd1 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//float rnd2 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
	//m_ubo.m_pack = Vector4{ rnd0, rnd1, rnd2, static_cast<float>(m_pRHI->GetCurrentFrameIndex()) };

	m_ubo.m_viewport.x = static_cast<uint32_t>(m_pRHI->GetSwapchainInfo().m_pViewport->m_x);
	m_ubo.m_viewport.y = static_cast<uint32_t>(m_pRHI->GetSwapchainInfo().m_pViewport->m_y);
	m_ubo.m_viewport.z = static_cast<uint32_t>(m_pRHI->GetSwapchainInfo().m_pViewport->m_width);
	m_ubo.m_viewport.w = static_cast<uint32_t>(m_pRHI->GetSwapchainInfo().m_pViewport->m_height);
	m_ubo.m_extent.x = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_width);
	m_ubo.m_extent.y = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_height);

	m_ubo.m_extent.z = g_runtimeGlobalContext.m_pRenderSystem->GetRenderCamera()->m_zNear;
	m_ubo.m_extent.w = g_runtimeGlobalContext.m_pRenderSystem->GetRenderCamera()->m_zFar;
	memcpy(m_pParticleComputeBufferMapped, &m_ubo, sizeof(m_ubo));
}

void ParticlePass::UpdateEmitterTransform()
{
	for (ST_ParticleEmitterTransformDesc& transformDesc : m_emitterTransformIndices)
	{
		size_t index = transformDesc.m_id;
		m_emitterBufferBatches[index].m_emitterDesc.m_position = transformDesc.m_position;
		m_emitterBufferBatches[index].m_emitterDesc.m_rotation = transformDesc.m_rotation;

		memcpy(m_emitterBufferBatches[index].m_pEmitterDescMapped, &m_emitterBufferBatches[index].m_emitterDesc, sizeof(ST_ParticleEmitterDesc));
	}
}

void ParticlePass::AllocateDescriptorSet()
{
	ST_RHIDescriptorSetAllocateInfo particleDescriptorSetAllocInfo;
	particleDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	particleDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();

	m_descriptorInfos.resize(3 * m_emitterCount);
	for (int eid = 0; eid < m_emitterCount; ++eid)
	{
		particleDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[0].m_pDescriptorSetLayout;
		particleDescriptorSetAllocInfo.m_descriptorSetCount = 1;
		particleDescriptorSetAllocInfo.m_pNext = NULL;

		if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&particleDescriptorSetAllocInfo, m_descriptorInfos[eid * 3].m_pDescriptorSet))
			throw std::runtime_error("allocate compute descriptor set");

		particleDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[1].m_pDescriptorSetLayout;
		particleDescriptorSetAllocInfo.m_descriptorSetCount = 1;
		particleDescriptorSetAllocInfo.m_pNext = NULL;

		if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&particleDescriptorSetAllocInfo, m_descriptorInfos[eid * 3 + 1].m_pDescriptorSet))
			LOG_INFO("allocate normal and depth descriptor set done");
	}
}

void ParticlePass::UpdateDescriptorSet()
{
	// 更新每一个发射器的描述符集
	for (int eid = 0; eid < m_emitterCount; ++eid)
	{
		// compute part
		{
			std::vector<ST_RHIWriteDescriptorSet> computeWriteDescriptorSets{ {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {} };

			ST_RHIDescriptorBufferInfo uniformbufferDescriptor = { m_pComputeUniformBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[0];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorset.m_dstBinding = 0;
				descriptorset.m_pBufferInfo = &uniformbufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo positionBufferDescriptor = { m_emitterBufferBatches[eid].m_pPositionDeviceBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[1];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 1;
				descriptorset.m_pBufferInfo = &positionBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo counterBufferDescriptor = { m_emitterBufferBatches[eid].m_pCounterDeviceBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[2];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 2;
				descriptorset.m_pBufferInfo = &counterBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo indirectArgumentBufferDescriptor = { m_emitterBufferBatches[eid].m_pIndirectDispatchArgumentBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[3];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 3;
				descriptorset.m_pBufferInfo = &indirectArgumentBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo aliveListBufferDescriptor = { m_emitterBufferBatches[eid].m_pAliveListBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[4];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 4;
				descriptorset.m_pBufferInfo = &aliveListBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo deadListBufferDescriptor = { m_emitterBufferBatches[eid].m_pDeadListBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[5];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 5;
				descriptorset.m_pBufferInfo = &deadListBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo aliveListNextBufferDescriptor = { m_emitterBufferBatches[eid].m_pAliveListNextBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[6];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 6;
				descriptorset.m_pBufferInfo = &aliveListNextBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo particleComponentResBufferDescriptor = { m_emitterBufferBatches[eid].m_pParticleComponentResBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[7];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 7;
				descriptorset.m_pBufferInfo = &particleComponentResBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo particleSceneUniformBufferDescriptor = { m_pSceneUniformBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[8];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorset.m_dstBinding = 8;
				descriptorset.m_pBufferInfo = &particleSceneUniformBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			ST_RHIDescriptorBufferInfo positionRenderBufferDescriptor = { m_emitterBufferBatches[eid].m_pPositionRenderBuffer, 0, RHI_WHOLE_SIZE };
			{
				ST_RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[9];
				descriptorset.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorset.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorset.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorset.m_dstBinding = 9;
				descriptorset.m_pBufferInfo = &positionRenderBufferDescriptor;
				descriptorset.m_descriptorCount = 1;
			}

			RHISampler* pSampler;
			ST_RHISamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.m_sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.m_maxAnisotropy = 1.0f;
			samplerCreateInfo.m_anisotropyEnable = true;
			samplerCreateInfo.m_magFilter = RHI_FILTER_LINEAR;
			samplerCreateInfo.m_minFilter = RHI_FILTER_LINEAR;
			samplerCreateInfo.m_mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.m_addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_mipLodBias = 0.0f;
			samplerCreateInfo.m_compareOp = RHI_COMPARE_OP_NEVER;
			samplerCreateInfo.m_minLod = 0.0f;
			samplerCreateInfo.m_maxLod = 0.0f;
			samplerCreateInfo.m_borderColor = RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

			if (RHI_SUCCESS != m_pRHI->CreateSampler(&samplerCreateInfo, pSampler))
			{
				throw std::runtime_error("create sampler error");
			}

			ST_RHIDescriptorImageInfo piccoloTextureImageInfo = {};
			piccoloTextureImageInfo.m_pSampler = pSampler;
			piccoloTextureImageInfo.m_pImageView = m_pPiccoloLogoTextureImageView;
			piccoloTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读

			{
				ST_RHIWriteDescriptorSet& descriptorSet = computeWriteDescriptorSets[10];
				descriptorSet.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSet.m_pDstSet = m_descriptorInfos[eid * 3].m_pDescriptorSet;
				descriptorSet.m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorSet.m_dstBinding = 10;
				descriptorSet.m_pImageInfo = &piccoloTextureImageInfo;
				descriptorSet.m_descriptorCount = 1;
			}

			// 更新描述符集数据
			m_pRHI->UpdateDescriptorSets(static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);
		}

		{
			// 写入的描述符集数据数组
			ST_RHIWriteDescriptorSet descriptorInputAttachmentWritesInfo[2] = { {}, {} };

			ST_RHIDescriptorImageInfo gbufferNormalDescriptorImageInfo = {};
			gbufferNormalDescriptorImageInfo.m_pSampler = nullptr;
			gbufferNormalDescriptorImageInfo.m_pImageView = m_pSrcNormalImageView;
			gbufferNormalDescriptorImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_GENERAL;	// 存储图像
			{

				ST_RHIWriteDescriptorSet& gbufferNormalDescriptorInputAttachmentWriteInfo = descriptorInputAttachmentWritesInfo[0];
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_pNext = NULL;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_pDstSet = m_descriptorInfos[eid * 3 + 1].m_pDescriptorSet;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_dstBinding = 0;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_dstArrayElement = 0;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_descriptorCount = 1;
				gbufferNormalDescriptorInputAttachmentWriteInfo.m_pImageInfo = &gbufferNormalDescriptorImageInfo;
			}

			RHISampler* sampler;
			ST_RHISamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.m_sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.m_maxAnisotropy = 1.0f;
			samplerCreateInfo.m_anisotropyEnable = true;
			samplerCreateInfo.m_magFilter = RHI_FILTER_NEAREST;
			samplerCreateInfo.m_minFilter = RHI_FILTER_NEAREST;
			samplerCreateInfo.m_mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.m_addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.m_mipLodBias = 0.0f;
			samplerCreateInfo.m_compareOp = RHI_COMPARE_OP_NEVER;
			samplerCreateInfo.m_minLod = 0.0f;
			samplerCreateInfo.m_maxLod = 0.0f;
			samplerCreateInfo.m_borderColor = RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			if (RHI_SUCCESS != m_pRHI->CreateSampler(&samplerCreateInfo, sampler))
			{
				throw std::runtime_error("create sampler error");
			}

			ST_RHIDescriptorImageInfo depthDescriptorImageInfo = {};
			depthDescriptorImageInfo.m_pSampler = sampler;
			depthDescriptorImageInfo.m_pImageView = m_pSrcDepthImageView;
			depthDescriptorImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 着色器只读

			{
				ST_RHIWriteDescriptorSet& depthDescriptorInputAttachmentWriteInfo = descriptorInputAttachmentWritesInfo[1];
				depthDescriptorInputAttachmentWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				depthDescriptorInputAttachmentWriteInfo.m_pNext = NULL;
				depthDescriptorInputAttachmentWriteInfo.m_pDstSet = m_descriptorInfos[eid * 3 + 1].m_pDescriptorSet;
				depthDescriptorInputAttachmentWriteInfo.m_dstBinding = 1;
				depthDescriptorInputAttachmentWriteInfo.m_dstArrayElement = 0;
				depthDescriptorInputAttachmentWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				depthDescriptorInputAttachmentWriteInfo.m_descriptorCount = 1;
				depthDescriptorInputAttachmentWriteInfo.m_pImageInfo = &depthDescriptorImageInfo;
			}

			// 更新第二个描述符集数据
			m_pRHI->UpdateDescriptorSets(sizeof(descriptorInputAttachmentWritesInfo) / sizeof(descriptorInputAttachmentWritesInfo[0]), descriptorInputAttachmentWritesInfo, 0, NULL);
		}
	}
}

void ParticlePass::SetupParticleDescriptorSet()
{
	// 更新第三个描述符集信息
	for (int eid = 0; eid < m_emitterCount; ++eid)
	{
		ST_RHIDescriptorSetAllocateInfo particleBillboardGlobalDescriptorSetAllocInfo;
		particleBillboardGlobalDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		particleBillboardGlobalDescriptorSetAllocInfo.m_pNext = NULL;
		particleBillboardGlobalDescriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
		particleBillboardGlobalDescriptorSetAllocInfo.m_descriptorSetCount = 1;
		particleBillboardGlobalDescriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[2].m_pDescriptorSetLayout;

		if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&particleBillboardGlobalDescriptorSetAllocInfo, m_descriptorInfos[eid * 3 + 2].m_pDescriptorSet))
		{
			throw std::runtime_error("allocate particle billboard global descriptor set");
		}

		ST_RHIDescriptorBufferInfo particleBillboardPerframeStorageBufferInfo = {};
		particleBillboardPerframeStorageBufferInfo.m_pBuffer = m_pParticleBillboardUniformBuffer;
		particleBillboardPerframeStorageBufferInfo.m_offset = 0;
		particleBillboardPerframeStorageBufferInfo.m_range = RHI_WHOLE_SIZE;

		ST_RHIDescriptorBufferInfo particleBillboardPerdrawcallStorageBufferInfo = {};
		particleBillboardPerdrawcallStorageBufferInfo.m_pBuffer = m_emitterBufferBatches[eid].m_pPositionRenderBuffer;
		particleBillboardPerdrawcallStorageBufferInfo.m_offset = 0;
		particleBillboardPerdrawcallStorageBufferInfo.m_range = RHI_WHOLE_SIZE;

		ST_RHIWriteDescriptorSet particleBillboardDescriptorWritesInfo[3];

		particleBillboardDescriptorWritesInfo[0].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		particleBillboardDescriptorWritesInfo[0].m_pNext = NULL;
		particleBillboardDescriptorWritesInfo[0].m_pDstSet = m_descriptorInfos[eid * 3 + 2].m_pDescriptorSet;
		particleBillboardDescriptorWritesInfo[0].m_dstBinding = 0;
		particleBillboardDescriptorWritesInfo[0].m_dstArrayElement = 0;
		particleBillboardDescriptorWritesInfo[0].m_descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		particleBillboardDescriptorWritesInfo[0].m_descriptorCount = 1;
		particleBillboardDescriptorWritesInfo[0].m_pBufferInfo = &particleBillboardPerframeStorageBufferInfo;

		particleBillboardDescriptorWritesInfo[1].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		particleBillboardDescriptorWritesInfo[1].m_pNext = NULL;
		particleBillboardDescriptorWritesInfo[1].m_pDstSet = m_descriptorInfos[eid * 3 + 2].m_pDescriptorSet;
		particleBillboardDescriptorWritesInfo[1].m_dstBinding = 1;
		particleBillboardDescriptorWritesInfo[1].m_dstArrayElement = 0;
		particleBillboardDescriptorWritesInfo[1].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		particleBillboardDescriptorWritesInfo[1].m_descriptorCount = 1;
		particleBillboardDescriptorWritesInfo[1].m_pBufferInfo = &particleBillboardPerdrawcallStorageBufferInfo;

		RHISampler* sampler;
		ST_RHISamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.m_sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.m_maxAnisotropy = 1.0f;
		samplerCreateInfo.m_anisotropyEnable = true;
		samplerCreateInfo.m_magFilter = RHI_FILTER_LINEAR;
		samplerCreateInfo.m_minFilter = RHI_FILTER_LINEAR;
		samplerCreateInfo.m_mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.m_addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.m_addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.m_addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.m_mipLodBias = 0.0f;
		samplerCreateInfo.m_compareOp = RHI_COMPARE_OP_NEVER;
		samplerCreateInfo.m_minLod = 0.0f;
		samplerCreateInfo.m_maxLod = 0.0f;
		samplerCreateInfo.m_borderColor = RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (RHI_SUCCESS != m_pRHI->CreateSampler(&samplerCreateInfo, sampler))
		{
			throw std::runtime_error("create sampler error");
		}

		ST_RHIDescriptorImageInfo particleTextureImageInfo = {};
		particleTextureImageInfo.m_pSampler = sampler;
		particleTextureImageInfo.m_pImageView = m_pParticleBillboardTextureImageView;
		particleTextureImageInfo.m_imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		particleBillboardDescriptorWritesInfo[2].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		particleBillboardDescriptorWritesInfo[2].m_pNext = NULL;
		particleBillboardDescriptorWritesInfo[2].m_pDstSet = m_descriptorInfos[eid * 3 + 2].m_pDescriptorSet;
		particleBillboardDescriptorWritesInfo[2].m_dstBinding = 2;
		particleBillboardDescriptorWritesInfo[2].m_dstArrayElement = 0;
		particleBillboardDescriptorWritesInfo[2].m_descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		particleBillboardDescriptorWritesInfo[2].m_descriptorCount = 1;
		particleBillboardDescriptorWritesInfo[2].m_pImageInfo = &particleTextureImageInfo;

		// 更新第三个描述符集数据
		m_pRHI->UpdateDescriptorSets(sizeof(particleBillboardDescriptorWritesInfo) / sizeof(particleBillboardDescriptorWritesInfo[0]), particleBillboardDescriptorWritesInfo, 0, NULL);
	}
}

void ParticleEmitterBufferBatch::FreeUpBatch(std::shared_ptr<RHI> pRHI)
{
	pRHI->FreeMemory(m_pCounterHostMemory);
	pRHI->FreeMemory(m_pPositionHostMemory);
	pRHI->FreeMemory(m_pPositionDeviceMemory);
	pRHI->FreeMemory(m_pCounterDeviceMemory);
	pRHI->FreeMemory(m_pIndirectDispatchArgumentMemory);
	pRHI->FreeMemory(m_pAliveListMemory);
	pRHI->FreeMemory(m_pAliveListNextMemory);
	pRHI->FreeMemory(m_pDeadListMemory);
	pRHI->FreeMemory(m_pParticleComponentResMemory);
	pRHI->FreeMemory(m_pPositionRenderMemory);

	pRHI->DestroyBuffer(m_pPositionRenderBuffer);
	pRHI->DestroyBuffer(m_pPositionDeviceBuffer);
	pRHI->DestroyBuffer(m_pPositionHostBuffer);
	pRHI->DestroyBuffer(m_pCounterDeviceBuffer);
	pRHI->DestroyBuffer(m_pCounterHostBuffer);
	pRHI->DestroyBuffer(m_pIndirectDispatchArgumentBuffer);
	pRHI->DestroyBuffer(m_pAliveListBuffer);
	pRHI->DestroyBuffer(m_pAliveListNextBuffer);
	pRHI->DestroyBuffer(m_pDeadListBuffer);
	pRHI->DestroyBuffer(m_pParticleComponentResBuffer);
}

NAMESPACE_XYH_END

