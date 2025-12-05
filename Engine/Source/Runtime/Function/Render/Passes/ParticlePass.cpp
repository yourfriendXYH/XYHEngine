#include "ParticlePass.h"
#include <Runtime\Core\Macro.h>
#include <ParticleKickoffComp.h>
#include <ParticleEmitComp.h>
#include <ParticleSimulateComp.h>
#include <ParticleBillboardVert.h>
#include <ParticleBillboardFrag.h>

#include <Runtime/Function/Render/RenderSystem.h>
#include <Runtime/Function/Render/RenderCamera.h>

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

void ParticlePass::CopyNormalAndDepthImage()
{
}

void ParticlePass::SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage)
{
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

void ParticlePass::Simulate()
{
}

void ParticlePass::SetRenderPassHandle(RHIRenderPass* pRenderPass)
{
}

void ParticlePass::UpdateAfterFramebufferRecreate()
{
}

void ParticlePass::Draw()
{
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
		m_pRHI->CreateGlobalImage(
			m_pParticleBillboardTextureImage,
			m_pParticleBillboardTextureImageView,
			m_particleBillboardTextureVmaAllocation,
			m_pParticleBillboardTextureResource->m_width,
			m_pParticleBillboardTextureResource->m_height,
			m_pParticleBillboardTextureResource->m_pixels,
			m_pParticleBillboardTextureResource->m_format);
	}

	// piccolo texture
	{
		std::shared_ptr<TextureData> m_pPiccoloLogoTextureResource = m_pRenderResource->LoadTexture(m_pParticleManager->GetGlobalParticleRes().m_piccoloLogoTexturePath, true);
		m_pRHI->CreateGlobalImage(
			m_pPiccoloLogoTextureImage,
			m_pPiccoloLogoTextureImageView,
			m_piccoloLogoTextureVmaAllocation,
			m_pPiccoloLogoTextureResource->m_width,
			m_pPiccoloLogoTextureResource->m_height,
			m_pPiccoloLogoTextureResource->m_pixels,
			m_pPiccoloLogoTextureResource->m_format);
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

	m_ubo.m_viewport.x = m_pRHI->GetSwapchainInfo().m_pViewport->m_x;
	m_ubo.m_viewport.y = m_pRHI->GetSwapchainInfo().m_pViewport->m_y;
	m_ubo.m_viewport.z = m_pRHI->GetSwapchainInfo().m_pViewport->m_width;
	m_ubo.m_viewport.w = m_pRHI->GetSwapchainInfo().m_pViewport->m_height;
	m_ubo.m_extent.x = m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_width;
	m_ubo.m_extent.y = m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_height;

	m_ubo.m_extent.z = g_runtimeGlobalContext.m_pRenderSystem->GetRenderCamera()->m_zNear;
	m_ubo.m_extent.w = g_runtimeGlobalContext.m_pRenderSystem->GetRenderCamera()->m_zFar;
	memcpy(m_pParticleComputeBufferMapped, &m_ubo, sizeof(m_ubo));
}

void ParticlePass::UpdateEmitterTransform()
{
	for (ST_ParticleEmitterTransformDesc& transformDesc : m_emitterTransformIndices)
	{
		int index = transformDesc.m_id;
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

			//RHIDescriptorBufferInfo aliveListNextBufferDescriptor = {
			//	m_emitter_buffer_batches[eid].m_alive_list_next_buffer, 0, RHI_WHOLE_SIZE };
			//{
			//	RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[6];
			//	descriptorset.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorset.dstSet = m_descriptor_infos[eid * 3].descriptor_set;
			//	descriptorset.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			//	descriptorset.dstBinding = 6;
			//	descriptorset.pBufferInfo = &aliveListNextBufferDescriptor;
			//	descriptorset.descriptorCount = 1;
			//}

			//RHIDescriptorBufferInfo particleComponentResBufferDescriptor = {
			//	m_emitter_buffer_batches[eid].m_particle_component_res_buffer, 0, RHI_WHOLE_SIZE };
			//{
			//	RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[7];
			//	descriptorset.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorset.dstSet = m_descriptor_infos[eid * 3].descriptor_set;
			//	descriptorset.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			//	descriptorset.dstBinding = 7;
			//	descriptorset.pBufferInfo = &particleComponentResBufferDescriptor;
			//	descriptorset.descriptorCount = 1;
			//}

			//RHIDescriptorBufferInfo particleSceneUniformBufferDescriptor = {
			//	m_scene_uniform_buffer, 0, RHI_WHOLE_SIZE };
			//{
			//	RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[8];
			//	descriptorset.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorset.dstSet = m_descriptor_infos[eid * 3].descriptor_set;
			//	descriptorset.descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//	descriptorset.dstBinding = 8;
			//	descriptorset.pBufferInfo = &particleSceneUniformBufferDescriptor;
			//	descriptorset.descriptorCount = 1;
			//}

			//RHIDescriptorBufferInfo positionRenderbufferDescriptor = {
			//	m_emitter_buffer_batches[eid].m_position_render_buffer, 0, RHI_WHOLE_SIZE };
			//{
			//	RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[9];
			//	descriptorset.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorset.dstSet = m_descriptor_infos[eid * 3].descriptor_set;
			//	descriptorset.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			//	descriptorset.dstBinding = 9;
			//	descriptorset.pBufferInfo = &positionRenderbufferDescriptor;
			//	descriptorset.descriptorCount = 1;
			//}

			//RHISampler* sampler;
			//RHISamplerCreateInfo samplerCreateInfo{};
			//samplerCreateInfo.sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			//samplerCreateInfo.maxAnisotropy = 1.0f;
			//samplerCreateInfo.anisotropyEnable = true;
			//samplerCreateInfo.magFilter = RHI_FILTER_LINEAR;
			//samplerCreateInfo.minFilter = RHI_FILTER_LINEAR;
			//samplerCreateInfo.mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
			//samplerCreateInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			//samplerCreateInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			//samplerCreateInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
			//samplerCreateInfo.mipLodBias = 0.0f;
			//samplerCreateInfo.compareOp = RHI_COMPARE_OP_NEVER;
			//samplerCreateInfo.minLod = 0.0f;
			//samplerCreateInfo.maxLod = 0.0f;
			//samplerCreateInfo.borderColor = RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

			//if (RHI_SUCCESS != m_rhi->createSampler(&samplerCreateInfo, sampler))
			//{
			//	throw std::runtime_error("create sampler error");
			//}

			//RHIDescriptorImageInfo piccolo_texture_image_info = {};
			//piccolo_texture_image_info.sampler = sampler;
			//piccolo_texture_image_info.imageView = m_piccolo_logo_texture_image_view;
			//piccolo_texture_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			//{
			//	RHIWriteDescriptorSet& descriptorset = computeWriteDescriptorSets[10];
			//	descriptorset.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorset.dstSet = m_descriptor_infos[eid * 3].descriptor_set;
			//	descriptorset.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//	descriptorset.dstBinding = 10;
			//	descriptorset.pImageInfo = &piccolo_texture_image_info;
			//	descriptorset.descriptorCount = 1;
			//}

			//m_rhi->updateDescriptorSets(static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);
		}

		//{
		//	RHIWriteDescriptorSet descriptor_input_attachment_writes_info[2] = { {}, {} };

		//	RHIDescriptorImageInfo gbuffer_normal_descriptor_image_info = {};
		//	gbuffer_normal_descriptor_image_info.sampler = nullptr;
		//	gbuffer_normal_descriptor_image_info.imageView = m_src_normal_image_view;
		//	gbuffer_normal_descriptor_image_info.imageLayout = RHI_IMAGE_LAYOUT_GENERAL;
		//	{

		//		RHIWriteDescriptorSet& gbuffer_normal_descriptor_input_attachment_write_info =
		//			descriptor_input_attachment_writes_info[0];
		//		gbuffer_normal_descriptor_input_attachment_write_info.sType =
		//			RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		gbuffer_normal_descriptor_input_attachment_write_info.pNext = NULL;
		//		gbuffer_normal_descriptor_input_attachment_write_info.dstSet =
		//			m_descriptor_infos[eid * 3 + 1].descriptor_set;
		//		gbuffer_normal_descriptor_input_attachment_write_info.dstBinding = 0;
		//		gbuffer_normal_descriptor_input_attachment_write_info.dstArrayElement = 0;
		//		gbuffer_normal_descriptor_input_attachment_write_info.descriptorType =
		//			RHI_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		//		gbuffer_normal_descriptor_input_attachment_write_info.descriptorCount = 1;
		//		gbuffer_normal_descriptor_input_attachment_write_info.pImageInfo =
		//			&gbuffer_normal_descriptor_image_info;
		//	}

		//	RHISampler* sampler;
		//	RHISamplerCreateInfo samplerCreateInfo{};
		//	samplerCreateInfo.sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//	samplerCreateInfo.maxAnisotropy = 1.0f;
		//	samplerCreateInfo.anisotropyEnable = true;
		//	samplerCreateInfo.magFilter = RHI_FILTER_NEAREST;
		//	samplerCreateInfo.minFilter = RHI_FILTER_NEAREST;
		//	samplerCreateInfo.mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
		//	samplerCreateInfo.addressModeU = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		//	samplerCreateInfo.addressModeV = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		//	samplerCreateInfo.addressModeW = RHI_SAMPLER_ADDRESS_MODE_REPEAT;
		//	samplerCreateInfo.mipLodBias = 0.0f;
		//	samplerCreateInfo.compareOp = RHI_COMPARE_OP_NEVER;
		//	samplerCreateInfo.minLod = 0.0f;
		//	samplerCreateInfo.maxLod = 0.0f;
		//	samplerCreateInfo.borderColor = RHI_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		//	if (RHI_SUCCESS != m_rhi->createSampler(&samplerCreateInfo, sampler))
		//	{
		//		throw std::runtime_error("create sampler error");
		//	}

		//	RHIDescriptorImageInfo depth_descriptor_image_info = {};
		//	depth_descriptor_image_info.sampler = sampler;
		//	depth_descriptor_image_info.imageView = m_src_depth_image_view;
		//	depth_descriptor_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//	{
		//		RHIWriteDescriptorSet& depth_descriptor_input_attachment_write_info =
		//			descriptor_input_attachment_writes_info[1];
		//		depth_descriptor_input_attachment_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//		depth_descriptor_input_attachment_write_info.pNext = NULL;
		//		depth_descriptor_input_attachment_write_info.dstSet =
		//			m_descriptor_infos[eid * 3 + 1].descriptor_set;
		//		depth_descriptor_input_attachment_write_info.dstBinding = 1;
		//		depth_descriptor_input_attachment_write_info.dstArrayElement = 0;
		//		depth_descriptor_input_attachment_write_info.descriptorType =
		//			RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//		depth_descriptor_input_attachment_write_info.descriptorCount = 1;
		//		depth_descriptor_input_attachment_write_info.pImageInfo = &depth_descriptor_image_info;
		//	}

		//	m_rhi->updateDescriptorSets(sizeof(descriptor_input_attachment_writes_info) /
		//		sizeof(descriptor_input_attachment_writes_info[0]),
		//		descriptor_input_attachment_writes_info,
		//		0,
		//		NULL);
		//}
	}
}

void ParticlePass::SetupParticleDescriptorSet()
{
}

NAMESPACE_XYH_END

