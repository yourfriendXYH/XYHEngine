#include "TestPass.h"
#include <Runtime/Function/Render/Interface/Vulkan/VulkanRHI.h>
#include <Runtime/Function/Render/RenderMesh.h>

NAMESPACE_XYH_BEGIN

namespace
{
	std::vector<ST_MeshVertex::ST_TestVertexInput> s_vertexDatas = {
	{ Vector2(-0.5, -0.5), Vector3(1.0, 1.0, 1.0) },
	{ Vector2(0.5, -0.5), Vector3(1.0, 0.0, 0.0) },
	{ Vector2(0.5, 0.5), Vector3(0.0, 1.0, 0.0) },
	{ Vector2(-0.5, 0.5), Vector3(0.0, 0.0, 1.0) }
	};

	std::vector<uint16_t> s_indexDatas = {
		0, 1, 3,
		1, 2, 3
	};
}


void TestPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	SetupRenderPass();

	SetupPipelines();

	SetupSwapchainFramebuffers();

	CreateVertexBuffer();

	CreateIndexBuffer();
}

void TestPass::Draw()
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(m_pRHI.get());
	// 开始渲染通道
	{
		ST_RHIRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.m_sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
		renderPassBeginInfo.m_pFramebuffer = m_swapchainFramebuffers[pVulkanRHI->m_currentSwapchainImageIndex];
		renderPassBeginInfo.m_renderArea.m_offset = { 0, 0 };
		renderPassBeginInfo.m_renderArea.m_extent = m_pRHI->GetSwapchainInfo().m_extent;

		// 附件清除值
		UN_RHIClearValue clearValues[1];
		clearValues[0].m_color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		renderPassBeginInfo.m_clearValueCount = 1;
		renderPassBeginInfo.m_pClearValues = clearValues;

		m_pRHI->CmdBeginRenderPassPFN(m_pRHI->GetCurrentCommandBuffer(), &renderPassBeginInfo, RHI_SUBPASS_CONTENTS_INLINE);
	}

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pRHI->PushEvent(m_pRHI->GetCurrentCommandBuffer(), "Mesh GBuffer", color);	// 开始

	// 绑定管线
	m_pRHI->CmdBindPipelinePFN(m_pRHI->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_renderPipelines[0].m_pipeline);
	m_pRHI->CmdSetViewportPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pViewport);
	m_pRHI->CmdSetScissorPFN(m_pRHI->GetCurrentCommandBuffer(), 0, 1, m_pRHI->GetSwapchainInfo().m_pScissor);

	RHIBuffer* pVertexBuffer[] = {
		m_pVertexBuffer
	};
	RHIDeviceSize offsets[] = { 0 };

	// 绑定顶点缓冲区
	m_pRHI->CmdBindVertexBuffersPFN(m_pRHI->GetCurrentCommandBuffer(), 0, sizeof(pVertexBuffer) / sizeof(pVertexBuffer[0]), pVertexBuffer, offsets);
	// 绑定索引缓冲区
	m_pRHI->CmdBindIndexBufferPFN(m_pRHI->GetCurrentCommandBuffer(), m_pIndexBuffer, 0, RHI_INDEX_TYPE_UINT16);

	// 绘制
	//vkCmdDraw(((VulkanCommandBuffer*)m_pRHI->GetCurrentCommandBuffer())->GetResource(), 4, 1, 0, 0);
	m_pRHI->CmdDrawIndexedPFN(m_pRHI->GetCurrentCommandBuffer(), s_indexDatas.size(), 1, 0, 0, 0);

	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());	// 结束

	// 结束
	m_pRHI->CmdEndRenderPassPFN(m_pRHI->GetCurrentCommandBuffer());
}

void TestPass::UpdateAfterFramebufferRecreate()
{
	// 销毁帧缓冲
	for (auto* pFramebuffer : m_swapchainFramebuffers)
	{
		m_pRHI->DestroyFramebuffer(pFramebuffer);
	}

	// 重新创建帧缓冲
	SetupSwapchainFramebuffers();
}

void TestPass::SetupAttachments()
{

}

void TestPass::SetupRenderPass()
{
	ST_RHIAttachmentDescription attachments[1] = {};

	ST_RHIAttachmentDescription& gbufferNormalAttachmentDescription = attachments[0];
	gbufferNormalAttachmentDescription.m_format = m_pRHI->GetSwapchainInfo().m_imageFormat;	// 图像格式
	gbufferNormalAttachmentDescription.m_samples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;	// 采样数
	gbufferNormalAttachmentDescription.m_loadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_CLEAR;	// 加载操作 清除
	gbufferNormalAttachmentDescription.m_storeOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_STORE;	// 存储操作 存储
	gbufferNormalAttachmentDescription.m_stencilLoadOp = ERHIAttachmentLoadOp::RHI_ATTACHMENT_LOAD_OP_DONT_CARE;	// 模板加载操作 不关心
	gbufferNormalAttachmentDescription.m_stencilStoreOp = ERHIAttachmentStoreOp::RHI_ATTACHMENT_STORE_OP_DONT_CARE;	// 模板存储操作 不关心
	gbufferNormalAttachmentDescription.m_initialLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_UNDEFINED;	// 初始布局 未定义
	gbufferNormalAttachmentDescription.m_finalLayout = ERHIImageLayout::RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// 出现源布局，通常用于交换链图像

	ST_RHIAttachmentReference basePassColorAttachmentsReference;	// 3个颜色附件引用
	basePassColorAttachmentsReference.m_attachment = 0;	// 地址偏移值
	basePassColorAttachmentsReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// 颜色附件最优

	//ST_RHIAttachmentReference basePassDepthAttachmentReference = {};	// 深度附件引用
	//basePassDepthAttachmentReference.m_attachment = static_cast<uint32_t>(&depthAttachmentDescription - attachments);	// 地址偏移值
	//basePassDepthAttachmentReference.m_layout = ERHIImageLayout::RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 深度模板附件最优

	ST_RHISubpassDescription subpassDescription{};
	subpassDescription.m_flags = 0;
	subpassDescription.m_pipelineBindPoint = ERHIPipelineBindPoint::RHI_PIPELINE_BIND_POINT_GRAPHICS;	// 图形管线绑定点
	subpassDescription.m_colorAttachmentCount = 1;	// 颜色附件数量
	subpassDescription.m_pColorAttachments = &basePassColorAttachmentsReference;	// 颜色附件引用
	//subpassDescription.m_pDepthStencilAttachment = &basePassDepthAttachmentReference;	// 深度附件引用
	//subpassDescription.m_preserveAttachmentCount = 0;
	//subpassDescription.m_pPreserveAttachments = nullptr;

	ST_RHISubpassDependency subpassDependency;
	subpassDependency.m_srcSubpass = RHI_SUBPASS_EXTERNAL;
	subpassDependency.m_dstSubpass = 0;
	subpassDependency.m_srcStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.m_dstStageMask = ERHIPipelineStageFlagBits::RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.m_srcAccessMask = 0;
	subpassDependency.m_dstAccessMask = ERHIAccessFlagBits::RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.m_dependencyFlags = 0; // NOT BY REGION

	ST_RHIRenderPassCreateInfo renderpassCreateInfo{};
	renderpassCreateInfo.m_sType = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.m_attachmentCount = 1;
	renderpassCreateInfo.m_pAttachments = attachments;
	renderpassCreateInfo.m_subpassCount = 1;
	renderpassCreateInfo.m_pSubpasses = &subpassDescription;
	renderpassCreateInfo.m_dependencyCount = 0;
	renderpassCreateInfo.m_pDependencies = nullptr;
	if (m_pRHI->CreateRenderPass(&renderpassCreateInfo, m_framebuffer.m_pRenderPass) != RHI_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}
}

void TestPass::SetupDescriptorSetLayout()
{
}

void TestPass::SetupPipelines()
{
	m_renderPipelines.resize(1);
	// DX_C++
	// DirectX12_learn
	std::vector<unsigned char> vsByteCode = VulkanRHI::ReadShaderByteCode("D:/DX_C++/XYHEngine/Engine/Shader/Generated/spv/testVkVert.spv");
	std::vector<unsigned char> fsByteCode = VulkanRHI::ReadShaderByteCode("D:/DX_C++/XYHEngine/Engine/Shader/Generated/spv/testVkFrag.spv");
	RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(vsByteCode);
	RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(fsByteCode);

	ST_RHIPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
	vertShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateInfo.m_module = pVertShaderModule;
	vertShaderStageCreateInfo.m_pName = "main";

	ST_RHIPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
	fragShaderStageCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.m_stage = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.m_module = pFragShaderModule;
	fragShaderStageCreateInfo.m_pName = "main";

	ST_RHIPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageCreateInfo,
		fragShaderStageCreateInfo
	};

	// 顶点布局属性
	auto vertexBindingDescriptions = ST_MeshVertex::GetBindingDescriptionsTest();
	auto vertexAttributeDescriptions = ST_MeshVertex::GetAttributeDescriptionsTest();
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
	rasterizationStateCreateInfo.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_CLOCKWISE;	// 顺时针为正面
	rasterizationStateCreateInfo.m_depthBiasEnable = RHI_FALSE;	// 是否开启深度偏移
	rasterizationStateCreateInfo.m_depthBiasConstantFactor = 0.0f;
	rasterizationStateCreateInfo.m_depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.m_depthBiasSlopeFactor = 0.0f;

	// 多重采样状态创建信息（抗锯齿的硬件加速器）
	ST_RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
	multisampleStateCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.m_sampleShadingEnable = RHI_FALSE;
	multisampleStateCreateInfo.m_rasterizationSamples = ERHISampleCountFlagBits::RHI_SAMPLE_COUNT_1_BIT;

	// 每个颜色附件的混合状态
	// src 和 dst 的定义
	// 在 Vulkan 的混合方程中：
	// src(Source) = 片段着色器输出的新颜色（即将要绘制的像素）
	// dst(Destination) = 帧缓冲区中已有的旧颜色（已经存在的像素）
	ST_RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};	// 每个颜色附件的混合状态
	colorBlendAttachments[0].m_colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
	colorBlendAttachments[0].m_blendEnable = RHI_FALSE;
	colorBlendAttachments[0].m_srcColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
	colorBlendAttachments[0].m_dstColorBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
	colorBlendAttachments[0].m_colorBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;
	colorBlendAttachments[0].m_srcAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ONE;
	colorBlendAttachments[0].m_dstAlphaBlendFactor = ERHIBlendFactor::RHI_BLEND_FACTOR_ZERO;
	colorBlendAttachments[0].m_alphaBlendOp = ERHIBlendOp::RHI_BLEND_OP_ADD;

	// 颜色混合状态创建信息（全局）
	ST_RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.m_sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.m_logicOpEnable = RHI_FALSE;	// 不启用
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
	depthStencilCreateInfo.m_depthTestEnable = RHI_FALSE;
	depthStencilCreateInfo.m_depthWriteEnable = RHI_FALSE;
	depthStencilCreateInfo.m_depthCompareOp = ERHICompareOp::RHI_COMPARE_OP_LESS;	// 通过深度测试的条件
	depthStencilCreateInfo.m_depthBoundsTestEnable = RHI_FALSE;
	depthStencilCreateInfo.m_stencilTestEnable = RHI_FALSE;

	// 创建渲染管线布局
	ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.m_setLayoutCount = 0;
	pipelineLayoutCreateInfo.m_pSetLayouts = nullptr;

	if (m_pRHI->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_renderPipelines[0].m_pipelineLayout) != RHI_SUCCESS)
	{
		throw std::runtime_error("create mesh gbuffer pipeline layout");
	}

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
	pipelineInfo.m_pLayout = m_renderPipelines[0].m_pipelineLayout;
	pipelineInfo.m_pRenderPass = m_framebuffer.m_pRenderPass;
	pipelineInfo.m_subpass = 0;
	pipelineInfo.m_pBasePipelineHandle = RHI_NULL_HANDLE;
	pipelineInfo.m_pDynamicState = &dynamicStateCreateInfo;

	if (RHI_SUCCESS != m_pRHI->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_renderPipelines[0].m_pipeline))
	{
		throw std::runtime_error("create mesh gbuffer graphics pipeline");
	}

	m_pRHI->DestroyShaderModule(pVertShaderModule);
	m_pRHI->DestroyShaderModule(pFragShaderModule);

}

void TestPass::SetupSwapchainFramebuffers()
{
	m_swapchainFramebuffers.resize(m_pRHI->GetSwapchainInfo().m_imageViews.size());

	// 为交换链的每个图像视图创建一个帧缓冲区
	// 3缓冲渲染
	for (size_t i = 0; i < m_pRHI->GetSwapchainInfo().m_imageViews.size(); i++)	// 目前是3个帧缓冲
	{
		RHIImageView* framebufferAttachmentsForImageView[1] = {
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

	if (1)
	{

	}
}

void TestPass::CreateVertexBuffer()
{
	RHIDeviceSize vertexBufferSize = sizeof(ST_MeshVertex::ST_TestVertexInput) * s_vertexDatas.size();

	RHIBuffer* pTempBuffer = nullptr;
	RHIDeviceMemory* pTempMemory = nullptr;
	m_pRHI->CreateBuffer(
		vertexBufferSize,
		RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		pTempBuffer,
		pTempMemory
	);

	void* pVertexMap = nullptr;
	m_pRHI->MapMemory(pTempMemory, 0, RHI_WHOLE_SIZE, 0, &pVertexMap);
	memcpy(pVertexMap, s_vertexDatas.data(), vertexBufferSize);
	m_pRHI->UnmapMemory(pTempMemory);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	ST_RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.m_usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

	bufferInfo.m_size = vertexBufferSize;
	m_pRHI->CreateBufferVMA(
		static_cast<VulkanRHI*>(m_pRHI.get())->m_assetsAllocator,
		&bufferInfo,
		&allocInfo,
		m_pVertexBuffer,
		&m_vertexBufferAllocation,
		nullptr
	);

	m_pRHI->CopyBuffer(pTempBuffer, m_pVertexBuffer, 0, 0, vertexBufferSize);

	m_pRHI->DestroyBuffer(pTempBuffer);
	m_pRHI->FreeMemory(pTempMemory);
}

void TestPass::CreateIndexBuffer()
{
	RHIDeviceSize indexBufferSize = sizeof(uint16_t) * s_indexDatas.size();

	RHIBuffer* pTempBuffer = nullptr;
	RHIDeviceMemory* pTempMemory = nullptr;

	m_pRHI->CreateBuffer(
		indexBufferSize,
		RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		pTempBuffer,
		pTempMemory
	);
	void* pIndexMap = nullptr;
	m_pRHI->MapMemory(pTempMemory, 0, RHI_WHOLE_SIZE, 0, &pIndexMap);
	memcpy(pIndexMap, s_indexDatas.data(), indexBufferSize);
	m_pRHI->UnmapMemory(pTempMemory);

	ST_RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.m_size = indexBufferSize;
	bufferInfo.m_usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	m_pRHI->CreateBufferVMA(
		static_cast<VulkanRHI*>(m_pRHI.get())->m_assetsAllocator,
		&bufferInfo,
		&allocInfo,
		m_pIndexBuffer,
		&m_indexBufferAllocation,
		nullptr
	);

	m_pRHI->CopyBuffer(pTempBuffer, m_pIndexBuffer, 0, 0, indexBufferSize);

	m_pRHI->DestroyBuffer(pTempBuffer);
	m_pRHI->FreeMemory(pTempMemory);
}

NAMESPACE_XYH_END

