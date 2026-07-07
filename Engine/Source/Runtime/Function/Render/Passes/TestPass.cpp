#include "TestPass.h"
#include <Runtime/Function/Render/Interface/OpenGL/OpenGLRHI.h>
#include <Runtime/Function/Render/Interface/Vulkan/VulkanRHI.h>
#include <Runtime/Function/Render/Interface/DX12/D3D12RHI.h>
#include <Runtime/Function/Render/RenderMesh.h>
#include <Runtime/Function/Render/Interface/OpenGL/Util.h>
#include <Runtime/Function/Render/Interface/OpenGL/OpenGLRHIResource.h>
#include <stbi/stb_image.h>

#include <random>


NAMESPACE_XYH_BEGIN

namespace
{
	std::vector<ST_MeshVertex::ST_TestVertexInput> s_vertexDatas = {
	{ Vector2(-1.0, -1.0), Vector3(1.0, 1.0, 1.0) },
	{ Vector2(1.0, -1.0), Vector3(1.0, 0.0, 0.0) },
	{ Vector2(1.0, 1.0), Vector3(0.0, 1.0, 0.0) },
	{ Vector2(-1.0, 1.0), Vector3(0.0, 0.0, 1.0) }
	};

	std::vector<uint16_t> s_indexDatas = {
		0, 1, 3,
		1, 2, 3
	};

#define _4MB 4194304
}

void TestPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
#ifdef USE_VULKAN
	SetupRenderPass();

	SetupDescriptorSetLayout();

	SetupDescriptorSet();

	SetupPipelines();

	SetupSwapchainFramebuffers();

	CreateVertexBuffer();

	CreateIndexBuffer();
#endif

#ifdef USE_D3D12

	D3D12RHI* pD3D12RHI = static_cast<D3D12RHI*>(m_pRHI.get());


	m_pRootSignature = pD3D12RHI->InitRootSignature();
	D3D12_SHADER_BYTECODE vs, gs, ps;
	pD3D12RHI->CreateShaderFromFile(L"Engine/Shader/hlsl/NDCTriangle.hlsl", "MainVS", "vs_5_1", &vs);
	pD3D12RHI->CreateShaderFromFile(L"Engine/Shader/hlsl/NDCTriangle.hlsl", "MainGS", "gs_5_1", &gs);
	pD3D12RHI->CreateShaderFromFile(L"Engine/Shader/hlsl/NDCTriangle.hlsl", "MainPS", "ps_5_1", &ps);
	m_pPSO = pD3D12RHI->CreatePSO(m_pRootSignature, vs, ps, gs);

	m_testMesh.InitFromFile(pD3D12RHI->GetGraphicsCommandList(), pD3D12RHI->GetDevice(), "Engine/Resource/Sphere.lhsm");
	m_vbos[0] = m_testMesh.m_vboView;

	// test constant buffer
	m_pTestConstantBuffer = D3D12Util::CreateConstantBufferObject(pD3D12RHI->GetDevice(), 65536);	// 1024 * 64( 4*4的矩阵-> 16个4字节(float) )
	//m_perframeStorageBufferObj.m_projViewMatrix = m_perframeStorageBufferObj.m_projViewMatrix.transpose();
	//D3D12Util::UpdateConstantBuffer(m_pTestConstantBuffer, &m_perframeStorageBufferObj.m_projViewMatrix, sizeof(Matrix4x4));
	
	unsigned int textureWidth = 256;
	unsigned int textureHeight = 256;
	unsigned char* pixelData = new unsigned char[textureWidth * textureHeight * 4];
	memset(pixelData, 0, textureWidth * textureHeight * 4);
	for (int y = 0; y < textureHeight; y++)
	{
		for (int x = 0; x < textureWidth; x++)
		{
			float radiusSqrt = float((x - textureWidth / 2.0) * (x - textureWidth / 2.0) + (y - textureHeight / 2.0) * (y - textureHeight / 2.0));
			if (radiusSqrt <= (textureWidth / 2.0) * (textureHeight / 2.0))
			{
				float radius = sqrtf(radiusSqrt);
				float alpha = radius / 128.0f;
				alpha = alpha > 1.0f ? 1.0 : alpha;
				alpha = 1 - alpha;
				alpha = powf(alpha, 2.0f);
				unsigned int pixelIndex = y * textureWidth + x;
				pixelData[pixelIndex * 4] = 255;
				pixelData[pixelIndex * 4 + 1] = 255;
				pixelData[pixelIndex * 4 + 2] = 255;
				pixelData[pixelIndex * 4 + 3] = unsigned char(255 * alpha);
			}
		}
	}

	m_pTestTexture1 = pD3D12RHI->CreateTexture2D(textureWidth, textureHeight, pixelData);
	delete[] pixelData;

	int imageWidth, imageHeight, imageChannel;
	stbi_uc* pixels = stbi_load("C://Users//Administrator//Desktop//huaban-6533916109.png", &imageWidth, &imageHeight, &imageChannel, 4);
	m_pTestTexture = pD3D12RHI->CreateTexture2D(imageWidth, imageHeight, pixels);
	delete[] pixels;

	ID3D12DescriptorHeap* srvHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescSRV{};
	descriptorHeapDescSRV.NumDescriptors = 3;
	descriptorHeapDescSRV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDescSRV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	pD3D12RHI->GetDevice()->CreateDescriptorHeap(&descriptorHeapDescSRV, IID_PPV_ARGS(&srvHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapPtr = srvHeap->GetCPUDescriptorHandleForHeapStart();
	pD3D12RHI->GetDevice()->CreateShaderResourceView(m_pTestTexture, &srvDesc, srvHeapPtr);
	// texture数组
	srvHeapPtr.ptr += pD3D12RHI->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pD3D12RHI->GetDevice()->CreateShaderResourceView(m_pTestTexture1, &srvDesc, srvHeapPtr);

	// struct buffer
	m_pStructBuffer = D3D12Util::CreateConstantBufferObject(pD3D12RHI->GetDevice(), 65536);
	struct ST_TestStructData
	{
		float r = 0.0f;
	};
	ST_TestStructData* pDatas = new ST_TestStructData[3000]; 
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	float randomValue = dist(gen);  // 范围 [-1, 1]
	for (size_t i = 0; i < 3000; i++)
	{
		pDatas[i].r = dist(gen) * 0.5 + 0.5;
	}
	D3D12Util::UpdateConstantBuffer(m_pStructBuffer, pDatas, sizeof(ST_TestStructData) * 3000);
	delete[] pDatas;

	// struct buffer
	D3D12_SHADER_RESOURCE_VIEW_DESC structBufferSrvDesc{};
	structBufferSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	structBufferSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	structBufferSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	structBufferSrvDesc.Buffer.FirstElement = 0;
	structBufferSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	structBufferSrvDesc.Buffer.NumElements = 3000;
	structBufferSrvDesc.Buffer.StructureByteStride = sizeof(ST_TestStructData);
	srvHeapPtr.ptr += pD3D12RHI->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pD3D12RHI->GetDevice()->CreateShaderResourceView(m_pStructBuffer, &structBufferSrvDesc, srvHeapPtr);

	m_descriptorHeaps.push_back(srvHeap);

#endif // USE_DX12

#ifdef USE_OPENGL

	OpenGLRHI* pOpenGLRHI = static_cast<OpenGLRHI*>(m_pRHI.get());

	m_FSQ.Init();

	size_t fileSize = 0;
	unsigned char* fileContent = LoadFileContent("Engine/Shader/glsl/fsqVS.glsl", fileSize);
	char* shaderCode = new char[fileSize + 1];
	memcpy(shaderCode, fileContent, fileSize);
	shaderCode[fileSize] = 0;
	GLuint vsShader = OpenGLUtil::CompileShader(GL_VERTEX_SHADER, shaderCode);
	delete[] shaderCode;
	delete[] fileContent;

	fileContent = LoadFileContent("Engine/Shader/glsl/fsqFS.glsl", fileSize);
	shaderCode = new char[fileSize + 1];
	memcpy(shaderCode, fileContent, fileSize);
	shaderCode[fileSize] = 0;
	GLuint fsShader = OpenGLUtil::CompileShader(GL_FRAGMENT_SHADER, shaderCode);
	delete[] shaderCode;
	delete[] fileContent;

	m_shaderProgramFSQ = OpenGLUtil::CreateProgram(vsShader, fsShader);

	pOpenGLRHI->CreateBufferObject(m_workArgs[0], GL_SHADER_STORAGE_BUFFER, _4MB, GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_workArgs[0])->GetResource(), "workArgs[0]");
	pOpenGLRHI->CreateBufferObject(m_workArgs[1], GL_SHADER_STORAGE_BUFFER, _4MB, GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_workArgs[1])->GetResource(), "workArgs[1]");

	pOpenGLRHI->CreateBufferObject(m_pGlobalConstants, GL_UNIFORM_BUFFER, 4096, GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_pGlobalConstants)->GetResource(), "GlobalConstants");

	pOpenGLRHI->CreateBufferObject(m_pMainAndPostNodeAndClusterBatches, GL_SHADER_STORAGE_BUFFER, _4MB, GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_pMainAndPostNodeAndClusterBatches)->GetResource(), "MainAndPostNodeAndClusterBatches");

	{
		size_t fileSize = 0;
		unsigned char* fileContent = LoadFileContent("Engine/Resource/HierarchyBuffer.data", fileSize);	// 加载 BVH
		pOpenGLRHI->CreateBufferObject(m_pBVH, GL_SHADER_STORAGE_BUFFER, fileSize, GL_STATIC_DRAW, fileContent);
		OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_pBVH)->GetResource(), "BVH");
		delete[] fileContent;
	}

	{
		size_t fileSize = 0;
		unsigned char* fileContent = LoadFileContent("Engine/Resource/mitsuba.nanitemesh", fileSize);	// 加载 Nanite Mesh
		pOpenGLRHI->CreateBufferObject(m_pNaniteMesh, GL_SHADER_STORAGE_BUFFER, fileSize, GL_STATIC_DRAW, fileContent);
		OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_pNaniteMesh)->GetResource(), "NaniteMesh");
		delete[] fileContent;
	}

	pOpenGLRHI->CreateBufferObject(m_pVisibleClusterSoftwareHardware, GL_SHADER_STORAGE_BUFFER, _4MB, GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, ((OpenGLBuffer*)m_pVisibleClusterSoftwareHardware)->GetResource(), "VisibleClusterSoftwareHardware");

	m_pVisualizationTexture = OpenGLUtil::CreateTexture2D(nullptr, 1280, 720, GL_RGBA32F, GL_RGBA);
	OpenGLUtil::SetObjectName(GL_TEXTURE, m_pVisualizationTexture->m_image, "VisualizationTexture");

	m_visBuffer64 = OpenGLUtil::CreateBufferObject(GL_SHADER_STORAGE_BUFFER, 1280 * 720 * sizeof(unsigned int), GL_STATIC_DRAW, nullptr);
	OpenGLUtil::SetObjectName(GL_BUFFER, m_visBuffer64, "VisBuffer64");
	OpenGLBuffer* pVisBuffer64 = new OpenGLBuffer;
	pVisBuffer64->SetResource(m_visBuffer64);

	int workGroupCountX = int(ceilf(pOpenGLRHI->GetSwapchainInfo().m_pViewport->m_width / 8.0f));
	int workGroupCountY = int(ceilf(pOpenGLRHI->GetSwapchainInfo().m_pViewport->m_height / 8.0f));

	m_pRasterClearPass = std::make_shared<RenderPass>(ERenderPassType::ERPT_COMPUTE, "RasterClear");
	if (nullptr != m_pRasterClearPass)
	{
		m_pRasterClearPass->SetComputeShader("Engine/Shader/glsl/RasterClear.glsl");
		m_pRasterClearPass->SetStorageBuffer(0, m_workArgs[0]);
		m_pRasterClearPass->SetStorageBuffer(1, m_workArgs[1]);
		m_pRasterClearPass->SetStorageBuffer(2, pVisBuffer64);
		m_pRasterClearPass->SetComputeDispatchArgs(workGroupCountX, workGroupCountY, 1);
		m_pRasterClearPass->Build();
	}

	m_pNodeAndClusterCullPass = std::make_shared<RenderPass>(ERenderPassType::ERPT_COMPUTE, "NodeAndClusterCull");
	if (nullptr != m_pNodeAndClusterCullPass)
	{
		m_pNodeAndClusterCullPass->SetComputeShader("Engine/Shader/glsl/NodeAndClusterCull.glsl");
		m_pNodeAndClusterCullPass->SetUniformBuffer(0, m_pGlobalConstants);
		m_pNodeAndClusterCullPass->SetStorageBuffer(1, m_pBVH);
		m_pNodeAndClusterCullPass->SetStorageBuffer(2, m_workArgs[0]);
		m_pNodeAndClusterCullPass->SetStorageBuffer(3, m_workArgs[1]);
		m_pNodeAndClusterCullPass->SetStorageBuffer(4, m_pMainAndPostNodeAndClusterBatches);
		m_pNodeAndClusterCullPass->SetComputeDispatchArgs(1, 1, 1);
		m_pNodeAndClusterCullPass->Build();
	}

	m_pClusterCullPass = std::make_shared<RenderPass>(ERenderPassType::ERPT_COMPUTE, "ClusterCull");
	if (nullptr != m_pNodeAndClusterCullPass)
	{
		m_pClusterCullPass->SetComputeShader("Engine/Shader/glsl/ClusterCull.glsl");
		m_pClusterCullPass->SetUniformBuffer(0, m_pGlobalConstants);
		m_pClusterCullPass->SetStorageBuffer(1, m_pNaniteMesh);
		m_pClusterCullPass->SetStorageBuffer(2, m_workArgs[0]);
		m_pClusterCullPass->SetStorageBuffer(3, m_pVisibleClusterSoftwareHardware);
		m_pClusterCullPass->SetStorageBuffer(4, m_pMainAndPostNodeAndClusterBatches);
		m_pClusterCullPass->SetComputeDispatchArgs(1, 1, 1);
		m_pClusterCullPass->Build();
	}

	// 
	m_pHardwareRasterizePass = std::make_shared<RenderPass>(ERenderPassType::ERPT_GRAPHICS, "HardwareRasterize");
	if (nullptr != m_pHardwareRasterizePass)
	{
		m_pHardwareRasterizePass->SetGraphicsShader("Engine/Shader/glsl/HardwareRasterizeVS.glsl", "Engine/Shader/glsl/HardwareRasterizeFS.glsl");
		m_pHardwareRasterizePass->SetUniformBuffer(0, m_pGlobalConstants);
		m_pHardwareRasterizePass->SetStorageBuffer(1, m_pNaniteMesh);
		m_pHardwareRasterizePass->SetStorageBuffer(2, m_pVisibleClusterSoftwareHardware);
		m_pHardwareRasterizePass->SetStorageBuffer(3, pVisBuffer64);
		m_pHardwareRasterizePass->Build(1280, 720);
	}

	// VisualizationPass 初始化
	m_pVisualizationPass = std::make_shared<RenderPass>(ERenderPassType::ERPT_COMPUTE, "Visualization");
	if (nullptr != m_pVisualizationPass)
	{
		m_pVisualizationPass->SetStorageBuffer(0, pVisBuffer64);
		m_pVisualizationPass->SetComputeImage(0, m_pVisualizationTexture);
		m_pVisualizationPass->SetComputeShader("Engine/Shader/glsl/VisualizationCS.glsl");
		m_pVisualizationPass->SetComputeDispatchArgs(workGroupCountX, workGroupCountY, 1);
		m_pVisualizationPass->Build();
	}

#endif // USE_OPENGL

}

void TestPass::Draw()
{
#ifdef USE_VULKAN
	VulkanDrawTest();
#endif // USE_VULKAN
#ifdef USE_D3D12
	D3D12DrawTest();
#endif // USE_DX12
#ifdef USE_OPENGL
	OpenGLDrawTest();
#endif // USE_OPENGL
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

void TestPass::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
{
	const RenderResource* vulkanResource = static_cast<const RenderResource*>(renderResource.get());
	if (nullptr != vulkanResource)
	{
		m_perframeStorageBufferObj.m_projViewMatrix = vulkanResource->m_meshPerframeStorageBufferObject.m_projViewMatrix;
	}
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
	m_descriptorInfos.resize(1);

	ST_RHIDescriptorSetLayoutBinding layoutBindings[2];

	ST_RHIDescriptorSetLayoutBinding& perFrameStorageBufferBinding = layoutBindings[0];
	perFrameStorageBufferBinding.m_binding = 0;
	perFrameStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perFrameStorageBufferBinding.m_descriptorCount = 1;
	perFrameStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
	perFrameStorageBufferBinding.m_pImmutableSamplers = nullptr;

	ST_RHIDescriptorSetLayoutBinding& perDrawcallStorageBufferBinding = layoutBindings[1];
	perDrawcallStorageBufferBinding.m_binding = 1;
	perDrawcallStorageBufferBinding.m_descriptorType = ERHIDescriptorType::RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perDrawcallStorageBufferBinding.m_descriptorCount = 1;
	perDrawcallStorageBufferBinding.m_stageFlags = ERHIShaderStageFlagBits::RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
	perDrawcallStorageBufferBinding.m_pImmutableSamplers = nullptr;

	ST_RHIDescriptorSetLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.m_pNext = NULL;
	layoutCreateInfo.m_flags = 0;
	layoutCreateInfo.m_bindingCount = (sizeof(layoutBindings) / sizeof(layoutBindings[0]));
	layoutCreateInfo.m_pBindings = layoutBindings;

	if (RHI_SUCCESS != m_pRHI->CreateDescriptorSetLayout(&layoutCreateInfo, m_descriptorInfos[0].m_pDescriptorSetLayout))
	{
		throw std::runtime_error("create mesh global layout");
	}
}

void TestPass::SetupDescriptorSet()
{
	m_pRHI->CreateBuffer(
		sizeof(ST_TestPerframeStorageBufferObject),
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_pPerframeStorageBuffer,
		m_pPerframeMemory
	);

	m_pRHI->CreateBuffer(
		sizeof(ST_TestPerDrawcallStorageBufferObject),
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_pPerDrawcallStorageBuffer,
		m_pPerDrawcallMemory
	);


	ST_RHIDescriptorSetAllocateInfo descriptorSetAllocInfo;
	descriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.m_pNext = NULL;
	descriptorSetAllocInfo.m_pDescriptorPool = m_pRHI->GetDescriptorPoor();
	descriptorSetAllocInfo.m_descriptorSetCount = 1;
	descriptorSetAllocInfo.m_pSetLayouts = &m_descriptorInfos[0].m_pDescriptorSetLayout;
	// 分配 描述符集
	if (RHI_SUCCESS != m_pRHI->AllocateDescriptorSets(&descriptorSetAllocInfo, m_descriptorInfos[0].m_pDescriptorSet))
	{
		throw std::runtime_error("allocate mesh global descriptor set");
	}

	ST_RHIDescriptorBufferInfo perframeStorageBufferInfo = {};
	perframeStorageBufferInfo.m_offset = 0;	// 这个偏移量加上dynamic_offset不应该大于缓冲区的大小
	perframeStorageBufferInfo.m_range = sizeof(ST_TestPerframeStorageBufferObject);	// 范围是指每次绘制调用时着色器实际使用的大小
	perframeStorageBufferInfo.m_pBuffer = m_pPerframeStorageBuffer;

	ST_RHIDescriptorBufferInfo perDrawcallStorageBufferInfo = {};
	perDrawcallStorageBufferInfo.m_offset = 0;	// 这个偏移量加上dynamic_offset不应该大于缓冲区的大小
	perDrawcallStorageBufferInfo.m_range = sizeof(ST_TestPerDrawcallStorageBufferObject);	// 范围是指每次绘制调用时着色器实际使用的大小
	perDrawcallStorageBufferInfo.m_pBuffer = m_pPerDrawcallStorageBuffer;

	ST_RHIWriteDescriptorSet descriptorWritesInfo[2];

	descriptorWritesInfo[0].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWritesInfo[0].m_pNext = nullptr;
	descriptorWritesInfo[0].m_pDstSet = m_descriptorInfos[0].m_pDescriptorSet;
	descriptorWritesInfo[0].m_dstBinding = 0;
	descriptorWritesInfo[0].m_dstArrayElement = 0;
	descriptorWritesInfo[0].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWritesInfo[0].m_descriptorCount = 1;
	descriptorWritesInfo[0].m_pBufferInfo = &perframeStorageBufferInfo;

	descriptorWritesInfo[1].m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWritesInfo[1].m_pNext = nullptr;
	descriptorWritesInfo[1].m_pDstSet = m_descriptorInfos[0].m_pDescriptorSet;
	descriptorWritesInfo[1].m_dstBinding = 1;
	descriptorWritesInfo[1].m_dstArrayElement = 0;
	descriptorWritesInfo[1].m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWritesInfo[1].m_descriptorCount = 1;
	descriptorWritesInfo[1].m_pBufferInfo = &perDrawcallStorageBufferInfo;

	m_pRHI->UpdateDescriptorSets((uint32_t)(sizeof(descriptorWritesInfo) / sizeof(descriptorWritesInfo[0])), descriptorWritesInfo, 0, nullptr);
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
	rasterizationStateCreateInfo.m_cullMode = ERHICullModeFlagBits::RHI_CULL_MODE_NONE;	// 背面剔除
	rasterizationStateCreateInfo.m_frontFace = ERHIFrontFace::RHI_FRONT_FACE_COUNTER_CLOCKWISE;	// 顺时针为正面
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
	RHIDescriptorSetLayout* descriptorSetLayouts[1] = {
		m_descriptorInfos[0].m_pDescriptorSetLayout
	};
	ST_RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.m_sType = ERHIStructureType::RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.m_setLayoutCount = 1;
	pipelineLayoutCreateInfo.m_pSetLayouts = descriptorSetLayouts;
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

void TestPass::VulkanDrawTest()
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
		clearValues[0].m_color = { {0.5f, 0.5f, 0.5f, 1.0f} };
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

	m_pRHI->CmdBindDescriptorSetsPFN(
		m_pRHI->GetCurrentCommandBuffer(),
		RHI_PIPELINE_BIND_POINT_GRAPHICS,
		m_renderPipelines[0].m_pipelineLayout,
		0,
		1,
		&m_descriptorInfos[0].m_pDescriptorSet,
		0,
		nullptr);

	// 绑定顶点缓冲区
	m_pRHI->CmdBindVertexBuffersPFN(m_pRHI->GetCurrentCommandBuffer(), 0, sizeof(pVertexBuffer) / sizeof(pVertexBuffer[0]), pVertexBuffer, offsets);
	// 绑定索引缓冲区
	m_pRHI->CmdBindIndexBufferPFN(m_pRHI->GetCurrentCommandBuffer(), m_pIndexBuffer, 0, RHI_INDEX_TYPE_UINT16);

	m_perDrawcallStorageBufferObj.m_modelMatrix = Matrix4x4(Vector3(0.f, 0.f, 0.f), Vector3(1.f, 1.f, 1.f), Quaternion(Radian(Degree(static_cast<float>(m_tempDegree))), Vector3(0.0, 0.0, 1.0)));
	m_tempDegree = (m_tempDegree + 1) % 360;

	// 转为列主序矩阵
	m_perframeStorageBufferObj.m_projViewMatrix = m_perframeStorageBufferObj.m_projViewMatrix.transpose();

	void* pMap = nullptr;
	m_pRHI->MapMemory(m_pPerframeMemory, 0, RHI_WHOLE_SIZE, 0, &pMap);
	*reinterpret_cast<ST_TestPerframeStorageBufferObject*>(pMap) = m_perframeStorageBufferObj;	// 赋值
	m_pRHI->UnmapMemory(m_pPerframeMemory);

	void* pMap1 = nullptr;
	m_pRHI->MapMemory(m_pPerDrawcallMemory, 0, RHI_WHOLE_SIZE, 0, &pMap1);
	*reinterpret_cast<ST_TestPerDrawcallStorageBufferObject*>(pMap1) = m_perDrawcallStorageBufferObj;	// 赋值
	m_pRHI->UnmapMemory(m_pPerDrawcallMemory);

	// 绘制
	//vkCmdDraw(((VulkanCommandBuffer*)m_pRHI->GetCurrentCommandBuffer())->GetResource(), 4, 1, 0, 0);
	m_pRHI->CmdDrawIndexedPFN(m_pRHI->GetCurrentCommandBuffer(), static_cast<uint32_t>(s_indexDatas.size()), 1, 0, 0, 0);

	m_pRHI->PopEvent(m_pRHI->GetCurrentCommandBuffer());	// 结束

	// 结束
	m_pRHI->CmdEndRenderPassPFN(m_pRHI->GetCurrentCommandBuffer());
}

void TestPass::D3D12DrawTest()
{
	DWORD currentTime = timeGetTime();

	D3D12RHI* pD3D12RHI = static_cast<D3D12RHI*>(m_pRHI.get());

	pD3D12RHI->BeginRenderToSwapChain(pD3D12RHI->GetGraphicsCommandList());

	ID3D12GraphicsCommandList* pCommandList = pD3D12RHI->GetGraphicsCommandList();
	pCommandList->SetPipelineState(m_pPSO);
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetDescriptorHeaps(m_descriptorHeaps.size(), m_descriptorHeaps.data());

	// 设置4个32bit -> 4个float
	m_testColor[0] = currentTime / 1000.0f;
	pCommandList->SetGraphicsRoot32BitConstants(0, 4, m_testColor, 0);
	// 设置constant buffer
	pCommandList->SetGraphicsRootConstantBufferView(1, m_pTestConstantBuffer->GetGPUVirtualAddress());

	pCommandList->SetGraphicsRootDescriptorTable(2, m_descriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());

	pCommandList->SetGraphicsRootShaderResourceView(3, m_pStructBuffer->GetGPUVirtualAddress());

	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, m_vbos.size(), m_vbos.data());

	m_perDrawcallStorageBufferObj.m_modelMatrix = Matrix4x4(Vector3(0.f, 0.f, 0.f), Vector3(1.f, 1.f, 1.f), Quaternion(Radian(Degree(static_cast<float>(m_tempDegree))), Vector3(0.0, 0.0, 1.0)));
	m_tempDegree = (m_tempDegree + 1) % 360;

	m_perframeStorageBufferObj.m_projViewMatrix = m_perframeStorageBufferObj.m_projViewMatrix.transpose();

	struct ST_GlobalMatrix
	{
		Matrix4x4 m_modelMatrix;
		Matrix4x4 m_viewProjMatrix;
		Matrix4x4 m_normalMatrix;
	}tempGlobalMatrix;
	tempGlobalMatrix.m_modelMatrix = m_perDrawcallStorageBufferObj.m_modelMatrix;
	tempGlobalMatrix.m_viewProjMatrix = m_perframeStorageBufferObj.m_projViewMatrix;
	tempGlobalMatrix.m_normalMatrix = m_perDrawcallStorageBufferObj.m_modelMatrix.inverse().transpose();	// 逆转置矩阵

	D3D12Util::UpdateConstantBuffer(m_pTestConstantBuffer, &tempGlobalMatrix, sizeof(ST_GlobalMatrix));

	for (auto iter = m_testMesh.m_subMeshData.begin(); iter != m_testMesh.m_subMeshData.end(); ++iter)
	{
		pCommandList->IASetIndexBuffer(&iter->second->m_iboView);
		pCommandList->DrawIndexedInstanced(iter->second->m_indexCount, 1, 0, 0, 0);
	}

	//pCommandList->DrawInstanced(3, 1, 0, 0);

	pD3D12RHI->EndRenderToSwapChain(pD3D12RHI->GetGraphicsCommandList());
}

void TestPass::OpenGLDrawTest()
{
	m_pRasterClearPass->Execute();

	m_pNodeAndClusterCullPass->Execute();

	m_pClusterCullPass->Execute();

	m_pHardwareRasterizePass->Execute(m_workArgs[0]);

	m_pVisualizationPass->Execute();

	OGL_CALL(glViewport(0, 0, 1280, 720));
	OGL_CALL(glScissor(0, 0, 1280, 720));
	OGL_CALL(glClearColor(0.1f, 0.4f, 0.6f, 1.0f));
	OGL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	OGL_CALL(glFrontFace(GL_CCW));

	OGL_CALL(glUseProgram(m_shaderProgramFSQ));

	OGL_CALL(glBindVertexArray(m_FSQ.m_VAO));

	OGL_CALL(glActiveTexture(GL_TEXTURE0));
	OGL_CALL(glBindTexture(GL_TEXTURE_2D, m_pVisualizationTexture->m_image));
	OGL_CALL(glUniform1i(0, 0));

	OGL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	OGL_CALL(glUseProgram(0));
}

NAMESPACE_XYH_END

