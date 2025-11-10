#include "VulkanRHI.h"
#include "../../WindowSystem.h"
#include "../../../../Core/Macro.h"
#include "VulkanUtil.h"
#include "VulkanRHIResource.h"
#include <array>
#include <set>
#include <string>
#include <windows.h>
#include <iostream>

NAMESPACE_XYH_BEGIN

void VulkanRHI::Initialize(ST_RHIInitInfo initInfo)
{
	m_pGLFWwindow = initInfo.m_pWindowSystem->GetWindow(); // 获取GLFW窗口指针
	std::array<int, 2> windowSize = initInfo.m_pWindowSystem->GetWindowSize(); // 获取窗口大小
	m_viewport = { 0.0f, 0.0f, static_cast<float>(windowSize[0]), static_cast<float>(windowSize[1]), 0.0f, 1.0f }; // 设置视口大小
	m_scissor = { { 0, 0 }, { (uint32_t)windowSize[0], (uint32_t)windowSize[1] } }; // 设置裁剪区域大小

#ifndef NDEBUG	// debug模式下启用验证层和调试工具标签
	m_enableValidationLayers = true;
	m_enableDebugUtilsLabel = true;
#else
	m_enable_validation_Layers = false;
	m_enable_debug_utils_label = false;
#endif

#if defined(__GNUC__) && defined(__MACH__)	// macOS平台下禁用点光源阴影
	m_enable_point_light_shadow = false;
#else
	m_enablePointLightShadow = true;
#endif

#if defined(__GNUC__)
	// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__linux__)
	char const* vk_layer_path = PICCOLO_XSTR(PICCOLO_VK_LAYER_PATH);
	setenv("VK_LAYER_PATH", vk_layer_path, 1);
#elif defined(__MACH__)
		// https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html
	char const* vk_layer_path = PICCOLO_XSTR(PICCOLO_VK_LAYER_PATH);
	char const* vk_icd_filenames = PICCOLO_XSTR(PICCOLO_VK_ICD_FILENAMES);
	setenv("VK_LAYER_PATH", vk_layer_path, 1);
	setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
#else
#error Unknown Platform
#endif
#elif defined(_MSC_VER)	// Microsoft Visual Studio平台下设置VK_LAYER_PATH环境变量
	// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
	char const* vk_layer_path = "D:\\DirectX12_learn\\XYHEngine\\Engine\\3rdparty\\VulkanSDK\\bin\\Win32";
	SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
	SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
#else
#error Unknown Compiler
#endif

	CreateInstance(); // 创建Vulkan实例

	InitializeDebugMessenger(); // 初始化调试消息处理器

	CreateWindowSurface(); // 创建窗口表面

	InitializePhysicalDevice(); // 初始化物理设备

	CreateLogicalDevice();	// 创建逻辑设备

	CreateCommandPool();	// 创建命令池

	CreateCommandBuffers();	// 创建命令缓冲区

	CreateDescriptorPool();	// 创建描述符池

	CreateSyncPrimitives();	// 创建同步原语

	CreateSwapChain();	// 创建交换链

	CreateSwapChainImageViews();	// 创建交换链图像视图

	CreateFramebufferImageAndViews();	// 创建帧缓冲图像和视图

	CreateAssetAllocator();	// 创建资源分配器
}

void VulkanRHI::PrepareContext()
{
	// 设置当前命令缓冲区
	m_vkCurrentCommandBuffer = m_vkCommandBuffers[m_currentFrameIndex];
	((VulkanCommandBuffer*)m_pCurrentCommandBuffer)->SetResource(m_vkCurrentCommandBuffer);
}

bool VulkanRHI::IsPointLightShadowEnabled()
{
	return false;
}

bool VulkanRHI::AllocateCommandBuffers(const ST_RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer*& pCommandBuffers)
{
	return false;
}

bool VulkanRHI::AllocateDescriptorSets(const ST_RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet*& pDescriptorSets)
{
	return false;
}

void VulkanRHI::CreateSwapChain()
{
	// 查询该物理设备的所有支持项
	ST_SwapChainSupportDetails swapchainSupportDetails = QuerySwapChainSupport(m_physicalDevice);

	// choose the best or fitting format
	VkSurfaceFormatKHR chosenSurfaceFormat = ChooseSwapchainSurfaceFormatFromDetails(swapchainSupportDetails.formats);

	// choose the best or fitting present mode
	VkPresentModeKHR chosenPresentMode = ChooseSwapchainPresentModeFromDetails(swapchainSupportDetails.presentModes);

	// 选择最合适的屏幕大小
	VkExtent2D chosenExtent = ChooseSwapchainExtentFromDetails(swapchainSupportDetails.capabilities);

	uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
	if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount)
	{
		imageCount = swapchainSupportDetails.capabilities.maxImageCount;
	}

	// 创建交换链的配置信息
	// KHR	Khronos 官方批准的扩展，跨厂商支持。
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;	// 交换链关联的表面
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = chosenSurfaceFormat.format;
	createInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
	createInfo.imageExtent = chosenExtent;	// 交换链的宽度和高度
	createInfo.imageArrayLayers = 1;	// 图像的层数，通常为1
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// 图像的使用方式，通常为颜色附件
	// 队列族索引
	uint32_t queueFamilyIndices[] = { m_queueIndices.m_graphicsFamily.value(), m_queueIndices.m_presentFamily.value() };
	// 如果图形队列族和呈现队列族不同
	if (m_queueIndices.m_graphicsFamily != m_queueIndices.m_presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	// 共享模式为并发
		createInfo.queueFamilyIndexCount = 2;	// 队列族索引的数量
		createInfo.pQueueFamilyIndices = queueFamilyIndices;	// 队列族索引数组
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	// 共享模式为独占
	}
	createInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;	// 交换链的当前变换
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// 合成Alpha通道的方式，通常为不透明
	createInfo.presentMode = chosenPresentMode;	// 交换链的呈现模式
	createInfo.clipped = VK_TRUE;	// 是否裁剪交换链图像
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// 创建交换链
	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
	{
		LOG_ERROR("vk create swapchain khr");
	}

	// 获取交换链图像
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

	// 设置交换链描述
	m_swapchainImageFormat = (ERHIFormat)chosenSurfaceFormat.format;
	m_swapchainExtent.m_height = chosenExtent.height;
	m_swapchainExtent.m_width = chosenExtent.width;

	m_scissor = { {0, 0}, {m_swapchainExtent.m_width, m_swapchainExtent.m_height} };	// 设置裁剪区域大小
}

void VulkanRHI::RecreateSwapChain()
{
}

void VulkanRHI::CreateSwapChainImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());

	// 遍历交换链图像，创建图像视图
	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageView vkImageView;
		// 创建图像视图
		vkImageView = VulkanUtil::CreateImageView(m_device, m_swapchainImages[i], (VkFormat)m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
		m_swapchainImageViews[i] = new VulkanImageView();
		((VulkanImageView*)m_swapchainImageViews[i])->SetResource(vkImageView);
	}
}

void VulkanRHI::CreateFramebufferImageAndViews()
{
	if (nullptr == m_depthImage)
	{
		m_depthImage = new VulkanImage();
	}
	// 创建深度图像
	VulkanUtil::CreateImage(
		m_physicalDevice,
		m_device,
		m_swapchainExtent.m_width,
		m_swapchainExtent.m_height,
		(VkFormat)m_depthImageFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		((VulkanImage*)m_depthImage)->GetResource(),	// 给深度图像赋值
		m_depthImageMemory,
		0,
		1,
		1);

	if (nullptr == m_depthImageView)
	{
		m_depthImageView = new VulkanImageView();
	}
	// 创建深度图像视图
	VkImageView depthImageView = VulkanUtil::CreateImageView(m_device, ((VulkanImage*)m_depthImage)->GetResource(), (VkFormat)m_depthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
	((VulkanImageView*)m_depthImageView)->SetResource(depthImageView);
}

RHISampler* VulkanRHI::GetOrCreateDefaultSampler(ERHIDefaultSamplerType type)
{
	switch (type)
	{
	case XYH::Default_Sampler_Linear:
		if (m_pLinearSampler == nullptr)
		{
			m_pLinearSampler = new VulkanSampler();
			((VulkanSampler*)m_pLinearSampler)->SetResource(VulkanUtil::GetOrCreateLinearSampler(m_physicalDevice, m_device));
		}
		return m_pLinearSampler;
		break;

	case XYH::Default_Sampler_Nearest:
		if (m_pNearestSampler == nullptr)
		{
			m_pNearestSampler = new VulkanSampler();
			((VulkanSampler*)m_pNearestSampler)->SetResource(VulkanUtil::GetOrCreateNearestSampler(m_physicalDevice, m_device));
		}
		return m_pNearestSampler;
		break;

	default:
		return nullptr;
		break;
	}
}

RHISampler* VulkanRHI::GetOrCreateMipmapSampler(uint32_t width, uint32_t height)
{
	return nullptr;
}

RHIShader* VulkanRHI::CreateShaderModule(const std::vector<unsigned char>& shaderCode)
{
	RHIShader* shahder = new VulkanShader();

	VkShaderModule vk_shader = VulkanUtil::CreateShaderModule(m_device, shaderCode);

	((VulkanShader*)shahder)->SetResource(vk_shader);

	return shahder;
}

void VulkanRHI::CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory)
{
}

void VulkanRHI::CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data, int datasize)
{
}

bool VulkanRHI::CreateBufferVMA(VmaAllocator allocator, const ST_RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIBuffer*& pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo)
{
	return false;
}

bool VulkanRHI::CreateBufferWithAlignmentVMA(VmaAllocator allocator, const ST_RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIDeviceSize minAlignment, RHIBuffer*& pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo)
{
	return false;
}

void VulkanRHI::CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size)
{
}

void VulkanRHI::CreateImage(uint32_t imageWidth, uint32_t imageHeight, ERHIFormat format, ERHIImageTiling imageTiling, RHIImageUsageFlags imageUsageFlags, RHIMemoryPropertyFlags memoryPropertyFlags, RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags imageCreateFlags, uint32_t arrayLayers, uint32_t miplevels)
{
	VkImage vkImage;
	VkDeviceMemory vkDeviceMemory;
	// 创建并获取vulkan的图像和图像内存
	VulkanUtil::CreateImage(
		m_physicalDevice,
		m_device,
		imageWidth,
		imageHeight,
		(VkFormat)format,
		(VkImageTiling)imageTiling,
		(VkImageUsageFlags)imageUsageFlags,
		(VkMemoryPropertyFlags)memoryPropertyFlags,
		vkImage,
		vkDeviceMemory,
		(VkImageCreateFlags)imageCreateFlags,
		arrayLayers,
		miplevels);

	image = new VulkanImage();
	((VulkanImage*)image)->SetResource(vkImage);

	memory = new VulkanDeviceMemory();
	((VulkanDeviceMemory*)memory)->SetResource(vkDeviceMemory);
}

void VulkanRHI::CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags imageAspectFlags, ERHIImageViewType viewType, uint32_t layoutCount, uint32_t miplevels, RHIImageView*& imageView)
{
	imageView = new VulkanImageView();
	VkImage vkImage = ((VulkanImage*)image)->GetResource();
	// 创建图像视图并封装
	VkImageView vkImageView = VulkanUtil::CreateImageView(m_device, vkImage, (VkFormat)format, imageAspectFlags, (VkImageViewType)viewType, layoutCount, miplevels);
	((VulkanImageView*)imageView)->SetResource(vkImageView);
}

void VulkanRHI::CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, ERHIFormat texture_image_format, uint32_t miplevels)
{
}

void VulkanRHI::CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, ERHIFormat texture_image_format, uint32_t miplevels)
{
}

bool VulkanRHI::CreateCommandPool(const ST_RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool)
{
	return false;
}

bool VulkanRHI::CreateDescriptorPool(const ST_RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool)
{
	return false;
}

bool VulkanRHI::CreateDescriptorSetLayout(const ST_RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout)
{
	int descriptorSetLayoutBindingSize = pCreateInfo->m_bindingCount;	// 描述符集布局绑定数量
	std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindingList(descriptorSetLayoutBindingSize);

	// 计算采样器数量
	int samplerCount = 0;
	for (int i = 0; i < descriptorSetLayoutBindingSize; ++i)
	{
		const auto& rhiDescriptorSetLayoutBindingElement = pCreateInfo->m_pBindings[i];
		if (rhiDescriptorSetLayoutBindingElement.m_pImmutableSamplers != nullptr)
		{
			samplerCount += rhiDescriptorSetLayoutBindingElement.m_descriptorCount;
		}
	}

	std::vector<VkSampler> samplerList(samplerCount);
	int samplerCurrent = 0;

	for (int i = 0; i < descriptorSetLayoutBindingSize; ++i)
	{
		const auto& rhiDescriptorSetLayoutBindingElement = pCreateInfo->m_pBindings[i];
		auto& vkDescriptorSetLayoutBindingElement = vkDescriptorSetLayoutBindingList[i];

		//添加采样器
		vkDescriptorSetLayoutBindingElement.pImmutableSamplers = nullptr;
		if (rhiDescriptorSetLayoutBindingElement.m_pImmutableSamplers)
		{
			vkDescriptorSetLayoutBindingElement.pImmutableSamplers = &samplerList[samplerCurrent];
			for (uint32_t i = 0; i < rhiDescriptorSetLayoutBindingElement.m_descriptorCount; ++i)
			{
				const auto& rhiSamplerElement = rhiDescriptorSetLayoutBindingElement.m_pImmutableSamplers[i];
				auto& vkSamplerElement = samplerList[samplerCurrent];

				vkSamplerElement = ((VulkanSampler*)rhiSamplerElement)->GetResource();

				samplerCurrent++;
			};
		}

		vkDescriptorSetLayoutBindingElement.binding = rhiDescriptorSetLayoutBindingElement.m_binding;	// 绑定号。这个数字必须与着色器（GLSL/HLSL）中使用的绑定号完全匹配。
		vkDescriptorSetLayoutBindingElement.descriptorType = (VkDescriptorType)rhiDescriptorSetLayoutBindingElement.m_descriptorType;	// 指定描述符的类型。它决定了这个绑定点对应的是哪种资源
		vkDescriptorSetLayoutBindingElement.descriptorCount = rhiDescriptorSetLayoutBindingElement.m_descriptorCount;	// 指定该绑定点有多少个描述符。通常为 1。如果大于 1，则表示这是一个描述符数组。
		vkDescriptorSetLayoutBindingElement.stageFlags = rhiDescriptorSetLayoutBindingElement.m_stageFlags;	// 指定哪个着色器阶段可以访问这个描述符。
	};

	if (samplerCount != samplerCurrent)
	{
		LOG_ERROR("sampler_count != sampller_current");
		return false;
	}

	// 创建描述符集布局
	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = (VkStructureType)pCreateInfo->m_sType;	// 结构体的类型标识符，必须设置为 VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
	createInfo.pNext = (const void*)pCreateInfo->m_pNext;	// 指向扩展结构的指针。用于启用一些扩展功能，通常设置为 nullptr
	createInfo.flags = (VkDescriptorSetLayoutCreateFlags)pCreateInfo->m_flags;	// 用于控制描述符集布局创建的标志位。通常设置为 0 或保留供未来使用
	createInfo.bindingCount = pCreateInfo->m_bindingCount;	// 指定 pBindings 数组中有多少个 VkDescriptorSetLayoutBinding 结构体
	createInfo.pBindings = vkDescriptorSetLayoutBindingList.data();	// 指向 VkDescriptorSetLayoutBinding 结构体数组的指针。这是最重要的成员，它包含了所有绑定的具体定义

	pSetLayout = new VulkanDescriptorSetLayout();
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkResult result = vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &vkDescriptorSetLayout);
	((VulkanDescriptorSetLayout*)pSetLayout)->SetResource(vkDescriptorSetLayout);

	if (result == VK_SUCCESS)
	{
		return RHI_SUCCESS;
	}
	else
	{
		LOG_ERROR("vkCreateDescriptorSetLayout failed!");
		return false;
	}
}

bool VulkanRHI::CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence)
{
	return false;
}

bool VulkanRHI::CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer)
{
	// 创建帧缓冲
	int imageViewSize = pCreateInfo->m_attachmentCount;
	std::vector<VkImageView> vkImageViewList(imageViewSize);
	for (int i = 0; i < imageViewSize; ++i)
	{
		const auto& rhiImageViewElement = pCreateInfo->m_pAttachments[i];
		auto& vkImageViewElement = vkImageViewList[i];

		vkImageViewElement = ((VulkanImageView*)rhiImageViewElement)->GetResource();
	};

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = (VkStructureType)pCreateInfo->m_sType;
	createInfo.pNext = (const void*)pCreateInfo->m_pNext;
	createInfo.flags = (VkFramebufferCreateFlags)pCreateInfo->m_flags;
	createInfo.renderPass = ((VulkanRenderPass*)pCreateInfo->m_pRenderPass)->GetResource();
	createInfo.attachmentCount = pCreateInfo->m_attachmentCount;
	createInfo.pAttachments = vkImageViewList.data();
	createInfo.width = pCreateInfo->m_width;
	createInfo.height = pCreateInfo->m_height;
	createInfo.layers = pCreateInfo->m_layers;

	pFramebuffer = new VulkanFramebuffer();
	VkFramebuffer vkFramebuffer;
	VkResult result = vkCreateFramebuffer(m_device, &createInfo, nullptr, &vkFramebuffer);
	((VulkanFramebuffer*)pFramebuffer)->SetResource(vkFramebuffer);

	if (result == VK_SUCCESS)
	{
		return RHI_SUCCESS;
	}
	else
	{
		LOG_ERROR("vkCreateFramebuffer failed!");
		return false;
	}
}

bool VulkanRHI::CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfo, RHIPipeline*& pPipelines)
{
	int pipelineShaderStageCreateInfoSize = pCreateInfo->m_stageCount;
	std::vector<VkPipelineShaderStageCreateInfo> vkPipelineShaderStageCreateInfoList(pipelineShaderStageCreateInfoSize);

	int specializationMapEntrySizeTotal = 0;
	int specializationInfoTotal = 0;
	for (int i = 0; i < pipelineShaderStageCreateInfoSize; ++i)
	{
		const auto& rhiPipelineShaderStageCreateInfoElement = pCreateInfo->m_pStages[i];
		if (rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo != nullptr)
		{
			specializationInfoTotal++;
			specializationMapEntrySizeTotal += rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_mapEntryCount;
		}
	}
	std::vector<VkSpecializationInfo> vkSpecializationInfoList(specializationInfoTotal);
	std::vector<VkSpecializationMapEntry> vkSpecializationMapEntryList(specializationMapEntrySizeTotal);

	int specializationMapEntryCurrent = 0;
	int specializationInfoCurrent = 0;

	// 着色器阶段创建信息
	for (int i = 0; i < pipelineShaderStageCreateInfoSize; ++i)
	{
		const auto& rhiPipelineShaderStageCreateInfoElement = pCreateInfo->m_pStages[i];
		auto& vkPipelineShaderStageCreateInfoElement = vkPipelineShaderStageCreateInfoList[i];

		// 存在 特殊化信息
		if (rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo != nullptr)
		{
			vkPipelineShaderStageCreateInfoElement.pSpecializationInfo = &vkSpecializationInfoList[specializationInfoCurrent];

			VkSpecializationInfo& vkSpecializationInfo = vkSpecializationInfoList[specializationInfoCurrent];
			vkSpecializationInfo.mapEntryCount = rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_mapEntryCount;
			vkSpecializationInfo.pMapEntries = &vkSpecializationMapEntryList[specializationMapEntryCurrent];
			vkSpecializationInfo.dataSize = rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_dataSize;
			vkSpecializationInfo.pData = (const void*)rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_pData;

			// 特殊化映射条目
			for (uint32_t i = 0; i < rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_mapEntryCount; ++i)
			{
				const auto& rhiSpecializationMapEntryElement = rhiPipelineShaderStageCreateInfoElement.m_pSpecializationInfo->m_pMapEntries[i];
				auto& vkSpecializationMapEntryElement = vkSpecializationMapEntryList[specializationMapEntryCurrent];

				vkSpecializationMapEntryElement.constantID = rhiSpecializationMapEntryElement->m_constantID;
				vkSpecializationMapEntryElement.offset = rhiSpecializationMapEntryElement->m_offset;
				vkSpecializationMapEntryElement.size = rhiSpecializationMapEntryElement->m_size;

				specializationMapEntryCurrent++;
			};

			specializationInfoCurrent++;
		}
		else
		{
			vkPipelineShaderStageCreateInfoElement.pSpecializationInfo = nullptr;
		}
		vkPipelineShaderStageCreateInfoElement.sType = (VkStructureType)rhiPipelineShaderStageCreateInfoElement.m_sType;
		vkPipelineShaderStageCreateInfoElement.pNext = (const void*)rhiPipelineShaderStageCreateInfoElement.m_pNext;
		vkPipelineShaderStageCreateInfoElement.flags = (VkPipelineShaderStageCreateFlags)rhiPipelineShaderStageCreateInfoElement.m_flags;
		vkPipelineShaderStageCreateInfoElement.stage = (VkShaderStageFlagBits)rhiPipelineShaderStageCreateInfoElement.m_stage;
		vkPipelineShaderStageCreateInfoElement.module = ((VulkanShader*)rhiPipelineShaderStageCreateInfoElement.m_module)->GetResource();
		vkPipelineShaderStageCreateInfoElement.pName = rhiPipelineShaderStageCreateInfoElement.m_pName;
	};

	// 检查遍历次数是否能对上
	if (!((specializationMapEntrySizeTotal == specializationMapEntryCurrent) && (specializationInfoTotal == specializationInfoCurrent)))
	{
		LOG_ERROR("(specialization_map_entry_size_total == specialization_map_entry_current)&& (specialization_info_total == specialization_info_current)");
		return false;
	}

	// 顶点输入绑定描述
	int vertexInputBindingDescriptionSize = pCreateInfo->m_pVertexInputState->m_vertexBindingDescriptionCount;
	std::vector<VkVertexInputBindingDescription> vkVertexInputBindingDescriptionList(vertexInputBindingDescriptionSize);
	for (int i = 0; i < vertexInputBindingDescriptionSize; ++i)
	{
		const auto& rhiVertexInputBindingDescriptionElement = pCreateInfo->m_pVertexInputState->m_pVertexBindingDescriptions[i];
		auto& vkVertexInputBindingDescriptionElement = vkVertexInputBindingDescriptionList[i];

		vkVertexInputBindingDescriptionElement.binding = rhiVertexInputBindingDescriptionElement.m_binding;
		vkVertexInputBindingDescriptionElement.stride = rhiVertexInputBindingDescriptionElement.m_stride;
		vkVertexInputBindingDescriptionElement.inputRate = (VkVertexInputRate)rhiVertexInputBindingDescriptionElement.m_inputRate;
	};

	// 顶点输入属性描述
	int vertexInputAttributeDescriptionSize = pCreateInfo->m_pVertexInputState->m_vertexAttributeDescriptionCount;
	std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributeDescriptionList(vertexInputAttributeDescriptionSize);
	for (int i = 0; i < vertexInputAttributeDescriptionSize; ++i)
	{
		const auto& rhiVertexInputAttributeDescriptionElement = pCreateInfo->m_pVertexInputState->m_pVertexAttributeDescriptions[i];
		auto& vkVertexInputAttributeDescriptionElement = vkVertexInputAttributeDescriptionList[i];

		vkVertexInputAttributeDescriptionElement.location = rhiVertexInputAttributeDescriptionElement.m_location;
		vkVertexInputAttributeDescriptionElement.binding = rhiVertexInputAttributeDescriptionElement.m_binding;
		vkVertexInputAttributeDescriptionElement.format = (VkFormat)rhiVertexInputAttributeDescriptionElement.m_format;
		vkVertexInputAttributeDescriptionElement.offset = rhiVertexInputAttributeDescriptionElement.m_offset;
	};

	// 顶点输入状态创建信息
	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo{};
	vkPipelineVertexInputStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pVertexInputState->m_sType;
	vkPipelineVertexInputStateCreateInfo.pNext = (const void*)pCreateInfo->m_pVertexInputState->m_pNext;
	vkPipelineVertexInputStateCreateInfo.flags = (VkPipelineVertexInputStateCreateFlags)pCreateInfo->m_pVertexInputState->m_flags;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = pCreateInfo->m_pVertexInputState->m_vertexBindingDescriptionCount;
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescriptionList.data();
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = pCreateInfo->m_pVertexInputState->m_vertexAttributeDescriptionCount;
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescriptionList.data();

	// 输入装配状态创建信息
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo{};
	vkPipelineInputAssemblyStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pInputAssemblyState->m_sType;
	vkPipelineInputAssemblyStateCreateInfo.pNext = (const void*)pCreateInfo->m_pInputAssemblyState->m_pNext;
	vkPipelineInputAssemblyStateCreateInfo.flags = (VkPipelineInputAssemblyStateCreateFlags)pCreateInfo->m_pInputAssemblyState->m_flags;
	vkPipelineInputAssemblyStateCreateInfo.topology = (VkPrimitiveTopology)pCreateInfo->m_pInputAssemblyState->m_topology;
	vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = (VkBool32)pCreateInfo->m_pInputAssemblyState->m_primitiveRestartEnable;

	// 曲面细分状态创建信息
	const VkPipelineTessellationStateCreateInfo* vkPipelineTessellationStateCreateInfoPtr = nullptr;
	VkPipelineTessellationStateCreateInfo vkPipelineTessellationStateCreateInfo{};
	if (pCreateInfo->m_pTessellationState != nullptr)
	{
		vkPipelineTessellationStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pTessellationState->m_sType;
		vkPipelineTessellationStateCreateInfo.pNext = (const void*)pCreateInfo->m_pTessellationState->m_pNext;
		vkPipelineTessellationStateCreateInfo.flags = (VkPipelineTessellationStateCreateFlags)pCreateInfo->m_pTessellationState->m_flags;
		vkPipelineTessellationStateCreateInfo.patchControlPoints = pCreateInfo->m_pTessellationState->m_patchControlPoints;

		vkPipelineTessellationStateCreateInfoPtr = &vkPipelineTessellationStateCreateInfo;
	}

	//viewport
	int viewportSize = pCreateInfo->m_pViewportState->m_viewportCount;
	std::vector<VkViewport> vkViewportList(viewportSize);
	for (int i = 0; i < viewportSize; ++i)
	{
		const auto& rhiViewportElement = pCreateInfo->m_pViewportState->m_pViewports[i];
		auto& vkViewportElement = vkViewportList[i];

		vkViewportElement.x = rhiViewportElement.m_x;
		vkViewportElement.y = rhiViewportElement.m_y;
		vkViewportElement.width = rhiViewportElement.m_width;
		vkViewportElement.height = rhiViewportElement.m_height;
		vkViewportElement.minDepth = rhiViewportElement.m_minDepth;
		vkViewportElement.maxDepth = rhiViewportElement.m_maxDepth;
	};

	//rect_2d
	int rect2dSize = pCreateInfo->m_pViewportState->m_scissorCount;
	std::vector<VkRect2D> vkRect2dList(rect2dSize);
	for (int i = 0; i < rect2dSize; ++i)
	{
		const auto& rhiRect2dElement = pCreateInfo->m_pViewportState->m_pScissors[i];
		auto& vkRect2dElement = vkRect2dList[i];

		VkOffset2D offset2d{};
		offset2d.x = rhiRect2dElement.m_offset.m_x;
		offset2d.y = rhiRect2dElement.m_offset.m_y;

		VkExtent2D extend2d{};
		extend2d.width = rhiRect2dElement.m_extent.m_width;
		extend2d.height = rhiRect2dElement.m_extent.m_height;

		vkRect2dElement.offset = offset2d;
		vkRect2dElement.extent = extend2d;
	};

	// 视口状态创建信息
	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo{};
	vkPipelineViewportStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pViewportState->m_sType;
	vkPipelineViewportStateCreateInfo.pNext = (const void*)pCreateInfo->m_pViewportState->m_pNext;
	vkPipelineViewportStateCreateInfo.flags = (VkPipelineViewportStateCreateFlags)pCreateInfo->m_pViewportState->m_flags;
	vkPipelineViewportStateCreateInfo.viewportCount = pCreateInfo->m_pViewportState->m_viewportCount;
	vkPipelineViewportStateCreateInfo.pViewports = vkViewportList.data();
	vkPipelineViewportStateCreateInfo.scissorCount = pCreateInfo->m_pViewportState->m_scissorCount;
	vkPipelineViewportStateCreateInfo.pScissors = vkRect2dList.data();

	// 光栅化状态创建信息
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo{};
	vkPipelineRasterizationStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pRasterizationState->m_sType;
	vkPipelineRasterizationStateCreateInfo.pNext = (const void*)pCreateInfo->m_pRasterizationState->m_pNext;
	vkPipelineRasterizationStateCreateInfo.flags = (VkPipelineRasterizationStateCreateFlags)pCreateInfo->m_pRasterizationState->m_flags;
	vkPipelineRasterizationStateCreateInfo.depthClampEnable = (VkBool32)pCreateInfo->m_pRasterizationState->m_depthClampEnable;
	vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = (VkBool32)pCreateInfo->m_pRasterizationState->m_rasterizerDiscardEnable;
	vkPipelineRasterizationStateCreateInfo.polygonMode = (VkPolygonMode)pCreateInfo->m_pRasterizationState->m_polygonMode;
	vkPipelineRasterizationStateCreateInfo.cullMode = (VkCullModeFlags)pCreateInfo->m_pRasterizationState->m_cullMode;
	vkPipelineRasterizationStateCreateInfo.frontFace = (VkFrontFace)pCreateInfo->m_pRasterizationState->m_frontFace;
	vkPipelineRasterizationStateCreateInfo.depthBiasEnable = (VkBool32)pCreateInfo->m_pRasterizationState->m_depthBiasEnable;
	vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = pCreateInfo->m_pRasterizationState->m_depthBiasConstantFactor;
	vkPipelineRasterizationStateCreateInfo.depthBiasClamp = pCreateInfo->m_pRasterizationState->m_depthBiasClamp;
	vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = pCreateInfo->m_pRasterizationState->m_depthBiasSlopeFactor;
	vkPipelineRasterizationStateCreateInfo.lineWidth = pCreateInfo->m_pRasterizationState->m_lineWidth;

	// 多样本状态创建信息
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo{};
	vkPipelineMultisampleStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pMultisampleState->m_sType;
	vkPipelineMultisampleStateCreateInfo.pNext = (const void*)pCreateInfo->m_pMultisampleState->m_pNext;
	vkPipelineMultisampleStateCreateInfo.flags = (VkPipelineMultisampleStateCreateFlags)pCreateInfo->m_pMultisampleState->m_flags;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = (VkSampleCountFlagBits)pCreateInfo->m_pMultisampleState->m_rasterizationSamples;	// 指定每个像素的采样数量
	vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = (VkBool32)pCreateInfo->m_pMultisampleState->m_sampleShadingEnable;
	vkPipelineMultisampleStateCreateInfo.minSampleShading = pCreateInfo->m_pMultisampleState->m_minSampleShading;
	vkPipelineMultisampleStateCreateInfo.pSampleMask = (const RHISampleMask*)pCreateInfo->m_pMultisampleState->m_pSampleMask;
	vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = (VkBool32)pCreateInfo->m_pMultisampleState->m_alphaToCoverageEnable;
	vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = (VkBool32)pCreateInfo->m_pMultisampleState->m_alphaToOneEnable;

	// 深度模板状态创建信息
	VkStencilOpState stencilOpStateFront{};
	stencilOpStateFront.failOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_front.m_failOp;
	stencilOpStateFront.passOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_front.m_passOp;
	stencilOpStateFront.depthFailOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_front.m_depthFailOp;
	stencilOpStateFront.compareOp = (VkCompareOp)pCreateInfo->m_pDepthStencilState->m_front.m_compareOp;
	stencilOpStateFront.compareMask = pCreateInfo->m_pDepthStencilState->m_front.m_compareMask;
	stencilOpStateFront.writeMask = pCreateInfo->m_pDepthStencilState->m_front.m_writeMask;
	stencilOpStateFront.reference = pCreateInfo->m_pDepthStencilState->m_front.m_reference;

	VkStencilOpState stencilOpStateBack{};
	stencilOpStateBack.failOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_back.m_failOp;
	stencilOpStateBack.passOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_back.m_passOp;
	stencilOpStateBack.depthFailOp = (VkStencilOp)pCreateInfo->m_pDepthStencilState->m_back.m_depthFailOp;
	stencilOpStateBack.compareOp = (VkCompareOp)pCreateInfo->m_pDepthStencilState->m_back.m_compareOp;
	stencilOpStateBack.compareMask = pCreateInfo->m_pDepthStencilState->m_back.m_compareMask;
	stencilOpStateBack.writeMask = pCreateInfo->m_pDepthStencilState->m_back.m_writeMask;
	stencilOpStateBack.reference = pCreateInfo->m_pDepthStencilState->m_back.m_reference;

	VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo{};
	vkPipelineDepthStencilStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pDepthStencilState->m_sType;
	vkPipelineDepthStencilStateCreateInfo.pNext = (const void*)pCreateInfo->m_pDepthStencilState->m_pNext;
	vkPipelineDepthStencilStateCreateInfo.flags = (VkPipelineDepthStencilStateCreateFlags)pCreateInfo->m_pDepthStencilState->m_flags;
	vkPipelineDepthStencilStateCreateInfo.depthTestEnable = (VkBool32)pCreateInfo->m_pDepthStencilState->m_depthTestEnable;
	vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = (VkBool32)pCreateInfo->m_pDepthStencilState->m_depthWriteEnable;
	vkPipelineDepthStencilStateCreateInfo.depthCompareOp = (VkCompareOp)pCreateInfo->m_pDepthStencilState->m_depthCompareOp;
	vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = (VkBool32)pCreateInfo->m_pDepthStencilState->m_depthBoundsTestEnable;
	vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = (VkBool32)pCreateInfo->m_pDepthStencilState->m_stencilTestEnable;
	vkPipelineDepthStencilStateCreateInfo.front = stencilOpStateFront;
	vkPipelineDepthStencilStateCreateInfo.back = stencilOpStateBack;
	vkPipelineDepthStencilStateCreateInfo.minDepthBounds = pCreateInfo->m_pDepthStencilState->m_minDepthBounds;
	vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = pCreateInfo->m_pDepthStencilState->m_maxDepthBounds;

	// 颜色混合状态创建信息
	// 颜色混合附件状态
	int pipelineColorBlendAttachmentStateSize = pCreateInfo->m_pColorBlendState->m_attachmentCount;
	std::vector<VkPipelineColorBlendAttachmentState> vkPipelineColorBlendAttachmentStateList(pipelineColorBlendAttachmentStateSize);
	for (int i = 0; i < pipelineColorBlendAttachmentStateSize; ++i)
	{
		const auto& rhiPipelineColorBlendAttachmentStateElement = pCreateInfo->m_pColorBlendState->m_pAttachments[i];
		auto& vkPipelineColorBlendAttachmentStateElement = vkPipelineColorBlendAttachmentStateList[i];

		vkPipelineColorBlendAttachmentStateElement.blendEnable = (VkBool32)rhiPipelineColorBlendAttachmentStateElement.m_blendEnable;
		vkPipelineColorBlendAttachmentStateElement.srcColorBlendFactor = (VkBlendFactor)rhiPipelineColorBlendAttachmentStateElement.m_srcColorBlendFactor;
		vkPipelineColorBlendAttachmentStateElement.dstColorBlendFactor = (VkBlendFactor)rhiPipelineColorBlendAttachmentStateElement.m_dstColorBlendFactor;
		vkPipelineColorBlendAttachmentStateElement.colorBlendOp = (VkBlendOp)rhiPipelineColorBlendAttachmentStateElement.m_colorBlendOp;
		vkPipelineColorBlendAttachmentStateElement.srcAlphaBlendFactor = (VkBlendFactor)rhiPipelineColorBlendAttachmentStateElement.m_srcAlphaBlendFactor;
		vkPipelineColorBlendAttachmentStateElement.dstAlphaBlendFactor = (VkBlendFactor)rhiPipelineColorBlendAttachmentStateElement.m_dstAlphaBlendFactor;
		vkPipelineColorBlendAttachmentStateElement.alphaBlendOp = (VkBlendOp)rhiPipelineColorBlendAttachmentStateElement.m_alphaBlendOp;
		vkPipelineColorBlendAttachmentStateElement.colorWriteMask = (VkColorComponentFlags)rhiPipelineColorBlendAttachmentStateElement.m_colorWriteMask;
	};

	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo{};
	vkPipelineColorBlendStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pColorBlendState->m_sType;
	vkPipelineColorBlendStateCreateInfo.pNext = pCreateInfo->m_pColorBlendState->m_pNext;
	vkPipelineColorBlendStateCreateInfo.flags = pCreateInfo->m_pColorBlendState->m_flags;
	vkPipelineColorBlendStateCreateInfo.logicOpEnable = pCreateInfo->m_pColorBlendState->m_logicOpEnable;
	vkPipelineColorBlendStateCreateInfo.logicOp = (VkLogicOp)pCreateInfo->m_pColorBlendState->m_logicOp;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = pCreateInfo->m_pColorBlendState->m_attachmentCount;
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentStateList.data();
	for (int i = 0; i < 4; ++i)
	{
		vkPipelineColorBlendStateCreateInfo.blendConstants[i] = pCreateInfo->m_pColorBlendState->m_blendConstants[i];
	};

	// 管线动态状态创建信息
	//dynamic_state
	int dynamicStateSize = pCreateInfo->m_pDynamicState->m_dynamicStateCount;
	std::vector<VkDynamicState> vkDynamicStateList(dynamicStateSize);
	for (int i = 0; i < dynamicStateSize; ++i)
	{
		const auto& rhiDynamicStateElement = pCreateInfo->m_pDynamicState->m_pDynamicStates[i];
		auto& vkDynamicStateElement = vkDynamicStateList[i];

		vkDynamicStateElement = (VkDynamicState)rhiDynamicStateElement;
	};

	VkPipelineDynamicStateCreateInfo vkPipelineDynamicStateCreateInfo{};
	vkPipelineDynamicStateCreateInfo.sType = (VkStructureType)pCreateInfo->m_pDynamicState->m_sType;
	vkPipelineDynamicStateCreateInfo.pNext = pCreateInfo->m_pDynamicState->m_pNext;
	vkPipelineDynamicStateCreateInfo.flags = (VkPipelineDynamicStateCreateFlags)pCreateInfo->m_pDynamicState->m_flags;
	vkPipelineDynamicStateCreateInfo.dynamicStateCount = pCreateInfo->m_pDynamicState->m_dynamicStateCount;
	vkPipelineDynamicStateCreateInfo.pDynamicStates = vkDynamicStateList.data();

	// 图形管线创建信息
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = (VkStructureType)pCreateInfo->m_sType;
	createInfo.pNext = (const void*)pCreateInfo->m_pNext;
	createInfo.flags = (VkPipelineCreateFlags)pCreateInfo->m_flags;
	createInfo.stageCount = pCreateInfo->m_stageCount;
	createInfo.pStages = vkPipelineShaderStageCreateInfoList.data();
	createInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	createInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	createInfo.pTessellationState = vkPipelineTessellationStateCreateInfoPtr;
	createInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	createInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	createInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	createInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
	createInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	createInfo.pDynamicState = &vkPipelineDynamicStateCreateInfo;
	createInfo.layout = ((VulkanPipelineLayout*)pCreateInfo->m_pLayout)->GetResource();
	createInfo.renderPass = ((VulkanRenderPass*)pCreateInfo->m_pRenderPass)->GetResource();
	createInfo.subpass = pCreateInfo->m_subpass;
	if (pCreateInfo->m_pBasePipelineHandle != nullptr)
	{
		createInfo.basePipelineHandle = ((VulkanPipeline*)pCreateInfo->m_pBasePipelineHandle)->GetResource();
	}
	else
	{
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
	}
	createInfo.basePipelineIndex = pCreateInfo->m_basePipelineIndex;

	// 创建图形管线
	pPipelines = new VulkanPipeline();
	VkPipeline vkPipelines;
	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;	// 管线缓存
	if (pipelineCache != nullptr)
	{
		vkPipelineCache = ((VulkanPipelineCache*)pipelineCache)->GetResource();
	}
	VkResult result = vkCreateGraphicsPipelines(m_device, vkPipelineCache, createInfoCount, &createInfo, nullptr, &vkPipelines);
	((VulkanPipeline*)pPipelines)->SetResource(vkPipelines);

	if (result == VK_SUCCESS)
	{
		return RHI_SUCCESS;
	}
	else
	{
		LOG_ERROR("vkCreateGraphicsPipelines failed!");
		return false;
	}
}

bool VulkanRHI::CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfo, RHIPipeline*& pPipelines)
{
	return false;
}

bool VulkanRHI::CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout)
{
	//descriptor_set_layout
	int descriptorSetLayoutSize = pCreateInfo->m_setLayoutCount;
	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayoutList(descriptorSetLayoutSize);
	for (int i = 0; i < descriptorSetLayoutSize; ++i)
	{
		const auto& rhiDescriptorSetLayoutElement = pCreateInfo->m_pSetLayouts[i];
		auto& vkDescriptorSetLayoutEslement = vkDescriptorSetLayoutList[i];

		vkDescriptorSetLayoutEslement = ((VulkanDescriptorSetLayout*)rhiDescriptorSetLayoutElement)->GetResource();
	};

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = (VkStructureType)pCreateInfo->m_sType;
	createInfo.pNext = (const void*)pCreateInfo->m_pNext;
	createInfo.flags = (VkPipelineLayoutCreateFlags)pCreateInfo->m_flags;
	createInfo.setLayoutCount = pCreateInfo->m_setLayoutCount;	// 指定 pSetLayouts 数组中有多少个描述符集布局。
	createInfo.pSetLayouts = vkDescriptorSetLayoutList.data();	// 指向 VkDescriptorSetLayout 数组的指针。每个元素对应一个描述符集（set）。

	pPipelineLayout = new VulkanPipelineLayout();
	VkPipelineLayout vkPipelineLayout;
	VkResult result = vkCreatePipelineLayout(m_device, &createInfo, nullptr, &vkPipelineLayout);	// 创建管线布局
	((VulkanPipelineLayout*)pPipelineLayout)->SetResource(vkPipelineLayout);

	if (result == VK_SUCCESS)
	{
		return RHI_SUCCESS;
	}
	else
	{
		LOG_ERROR("vkCreatePipelineLayout failed!");
		return false;
	}
}

bool VulkanRHI::CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass)
{
	// 渲染通道中的附件
	std::vector<VkAttachmentDescription> vkAttachments(pCreateInfo->m_attachmentCount);
	for (uint32_t i = 0; i < pCreateInfo->m_attachmentCount; ++i)
	{
		const auto& rhiDesc = pCreateInfo->m_pAttachments[i];

		auto& vkDesc = vkAttachments[i];
		vkDesc.flags = (VkAttachmentDescriptionFlags)(rhiDesc).m_flags;	// 通常不需要设置
		vkDesc.format = (VkFormat)(rhiDesc).m_format;	// 附件的图像格式
		vkDesc.samples = (VkSampleCountFlagBits)(rhiDesc).m_samples;	// 指定附件的样本数量。这主要用于多重采样抗锯齿（MSAA）
		vkDesc.loadOp = (VkAttachmentLoadOp)(rhiDesc).m_loadOp;		// 开始时 对颜色或深度数据的操作
		vkDesc.storeOp = (VkAttachmentStoreOp)(rhiDesc).m_storeOp;	// 结束时 对颜色或深度数据的操作
		vkDesc.stencilLoadOp = (VkAttachmentLoadOp)(rhiDesc).m_stencilLoadOp;	// 模板格式时的操作
		vkDesc.stencilStoreOp = (VkAttachmentStoreOp)(rhiDesc).m_stencilStoreOp;
		vkDesc.initialLayout = (VkImageLayout)(rhiDesc).m_initialLayout;	// 指定渲染流程开始之前，图像所处的布局
		vkDesc.finalLayout = (VkImageLayout)(rhiDesc).m_finalLayout;	// 指定渲染流程结束之后，图像应该被转换到的布局
	}


	// 通道描述转换
	int totalAttachmentRefenrence = 0;	// 全部的附件数量
	for (uint32_t i = 0; i < pCreateInfo->m_subpassCount; ++i)
	{
		const auto& rhiDesc = pCreateInfo->m_pSubpasses[i];

		totalAttachmentRefenrence += rhiDesc.m_inputAttachmentCount; // pInputAttachments
		totalAttachmentRefenrence += rhiDesc.m_colorAttachmentCount; // pColorAttachments
		// 增加对应颜色附件的数量
		if (rhiDesc.m_pDepthStencilAttachment != nullptr)
		{
			totalAttachmentRefenrence += rhiDesc.m_colorAttachmentCount; // pDepthStencilAttachment
		}
		if (rhiDesc.m_pResolveAttachments != nullptr)
		{
			totalAttachmentRefenrence += rhiDesc.m_colorAttachmentCount; // pResolveAttachments
		}
	}

	std::vector<VkSubpassDescription> vkSubpassDescription(pCreateInfo->m_subpassCount);
	std::vector<VkAttachmentReference> vkAttachmentReference(totalAttachmentRefenrence);
	int currentAttachmentReference = 0;
	for (uint32_t i = 0; i < pCreateInfo->m_subpassCount; ++i)
	{
		const auto& rhiDesc = pCreateInfo->m_pSubpasses[i];
		auto& vkDesc = vkSubpassDescription[i];

		vkDesc.flags = (VkSubpassDescriptionFlags)(rhiDesc).m_flags;
		vkDesc.pipelineBindPoint = (VkPipelineBindPoint)(rhiDesc).m_pipelineBindPoint;
		vkDesc.preserveAttachmentCount = (rhiDesc).m_preserveAttachmentCount;	// 指定本子流程不直接使用，但需要保留其内容的附件
		vkDesc.pPreserveAttachments = (const uint32_t*)(rhiDesc).m_pPreserveAttachments;

		// 填充 输入附件
		vkDesc.inputAttachmentCount = (rhiDesc).m_inputAttachmentCount;
		vkDesc.pInputAttachments = &vkAttachmentReference[currentAttachmentReference];
		for (uint32_t i = 0; i < (rhiDesc).m_inputAttachmentCount; i++)
		{
			const auto& rhiAttachmentReferenceInput = (rhiDesc).m_pInputAttachments[i];
			auto& vkAttachmentReferenceInput = vkAttachmentReference[currentAttachmentReference];

			vkAttachmentReferenceInput.attachment = rhiAttachmentReferenceInput.m_attachment;
			vkAttachmentReferenceInput.layout = (VkImageLayout)(rhiAttachmentReferenceInput.m_layout);

			currentAttachmentReference += 1;
		};

		// 填充 颜色附件
		vkDesc.colorAttachmentCount = (rhiDesc).m_colorAttachmentCount;
		vkDesc.pColorAttachments = &vkAttachmentReference[currentAttachmentReference];
		for (uint32_t i = 0; i < (rhiDesc).m_colorAttachmentCount; ++i)
		{
			const auto& rhiAttachmentReferenceColor = (rhiDesc).m_pColorAttachments[i];
			auto& vkAttachmentReferenceColor = vkAttachmentReference[currentAttachmentReference];

			vkAttachmentReferenceColor.attachment = rhiAttachmentReferenceColor.m_attachment;
			vkAttachmentReferenceColor.layout = (VkImageLayout)(rhiAttachmentReferenceColor.m_layout);

			currentAttachmentReference += 1;
		};

		// 填充 解决附件
		if (rhiDesc.m_pResolveAttachments != nullptr)
		{
			vkDesc.pResolveAttachments = &vkAttachmentReference[currentAttachmentReference];	// 用于多重采样抗锯齿（MSAA） 的解析操作
			for (uint32_t i = 0; i < (rhiDesc).m_colorAttachmentCount; ++i)
			{
				const auto& rhiAttachmentReferenceResolve = (rhiDesc).m_pResolveAttachments[i];
				auto& vkAttachmentReferenceResolve = vkAttachmentReference[currentAttachmentReference];

				vkAttachmentReferenceResolve.attachment = rhiAttachmentReferenceResolve.m_attachment;
				vkAttachmentReferenceResolve.layout = (VkImageLayout)(rhiAttachmentReferenceResolve.m_layout);

				currentAttachmentReference += 1;
			};
		}

		// 填充 深度模板附件
		if (rhiDesc.m_pDepthStencilAttachment != nullptr)
		{
			vkDesc.pDepthStencilAttachment = &vkAttachmentReference[currentAttachmentReference];	// 指定用于深度和模板测试的附件
			for (uint32_t i = 0; i < (rhiDesc).m_colorAttachmentCount; ++i)
			{
				const auto& rhiAttachmentReferenceDepth = (rhiDesc).m_pDepthStencilAttachment[i];
				auto& vkAttachmentReferenceDepth = vkAttachmentReference[currentAttachmentReference];

				vkAttachmentReferenceDepth.attachment = rhiAttachmentReferenceDepth.m_attachment;
				vkAttachmentReferenceDepth.layout = (VkImageLayout)(rhiAttachmentReferenceDepth.m_layout);

				currentAttachmentReference += 1;
			};
		};
	}
	// 判断附件数量是否对应
	if (currentAttachmentReference != totalAttachmentRefenrence)
	{
		LOG_ERROR("currentAttachmentRefence != totalAttachmentRefenrence");
		return false;
	}

	// 通道依赖转换
	std::vector<VkSubpassDependency> vkSubpassDepandencies(pCreateInfo->m_dependencyCount);
	for (uint32_t i = 0; i < pCreateInfo->m_dependencyCount; ++i)
	{
		const auto& rhiDesc = pCreateInfo->m_pDependencies[i];
		auto& vkDesc = vkSubpassDepandencies[i];

		vkDesc.srcSubpass = rhiDesc.m_srcSubpass;
		vkDesc.dstSubpass = rhiDesc.m_dstSubpass;
		vkDesc.srcStageMask = (VkPipelineStageFlags)(rhiDesc).m_srcStageMask;
		vkDesc.dstStageMask = (VkPipelineStageFlags)(rhiDesc).m_dstStageMask;
		vkDesc.srcAccessMask = (VkAccessFlags)(rhiDesc).m_srcAccessMask;
		vkDesc.dstAccessMask = (VkAccessFlags)(rhiDesc).m_dstAccessMask;
		vkDesc.dependencyFlags = (VkDependencyFlags)(rhiDesc).m_dependencyFlags;
	};

	// 创建渲染通道
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = (VkStructureType)pCreateInfo->m_sType;
	createInfo.pNext = (const void*)pCreateInfo->m_pNext;
	createInfo.flags = (VkRenderPassCreateFlags)pCreateInfo->m_flags;
	createInfo.attachmentCount = pCreateInfo->m_attachmentCount;
	createInfo.pAttachments = vkAttachments.data();
	createInfo.subpassCount = pCreateInfo->m_subpassCount;
	createInfo.pSubpasses = vkSubpassDescription.data();
	createInfo.dependencyCount = pCreateInfo->m_dependencyCount;
	createInfo.pDependencies = vkSubpassDepandencies.data();

	pRenderPass = new VulkanRenderPass();
	VkRenderPass vkRenderPass;
	VkResult result = vkCreateRenderPass(m_device, &createInfo, nullptr, &vkRenderPass);
	((VulkanRenderPass*)pRenderPass)->SetResource(vkRenderPass);

	if (result == VK_SUCCESS)
	{
		return RHI_SUCCESS;
	}
	else
	{
		LOG_ERROR("vkCreateRenderPass failed!");
		return false;
	}
	return false;
}

bool VulkanRHI::CreateSampler(const ST_RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler)
{
	return false;
}

bool VulkanRHI::CreateRHISemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore)
{
	return false;
}

bool VulkanRHI::WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout)
{
	return false;
}

bool VulkanRHI::ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences)
{
	return false;
}

bool VulkanRHI::ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags)
{
	return false;
}

bool VulkanRHI::BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo)
{
	return false;
}

bool VulkanRHI::EndCommandBufferPFN(RHICommandBuffer* commandBuffer)
{
	return false;
}

void VulkanRHI::CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const ST_RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents)
{
	VkOffset2D offset2d{};
	offset2d.x = pRenderPassBegin->m_renderArea.m_offset.m_x;
	offset2d.y = pRenderPassBegin->m_renderArea.m_offset.m_y;

	VkExtent2D extent2d{};
	extent2d.width = pRenderPassBegin->m_renderArea.m_extent.m_width;
	extent2d.height = pRenderPassBegin->m_renderArea.m_extent.m_height;

	VkRect2D rect2d{};
	rect2d.offset = offset2d;
	rect2d.extent = extent2d;

	//clear_values
	int clearValueSize = pRenderPassBegin->m_clearValueCount;
	std::vector<VkClearValue> vkClearValueList(clearValueSize);
	for (int i = 0; i < clearValueSize; ++i)
	{
		const auto& rhiClearValueElement = pRenderPassBegin->m_pClearValues[i];
		auto& vkClearValueElement = vkClearValueList[i];

		VkClearColorValue vkClearColorValue;
		vkClearColorValue.float32[0] = rhiClearValueElement.m_color.m_float32[0];
		vkClearColorValue.float32[1] = rhiClearValueElement.m_color.m_float32[1];
		vkClearColorValue.float32[2] = rhiClearValueElement.m_color.m_float32[2];
		vkClearColorValue.float32[3] = rhiClearValueElement.m_color.m_float32[3];
		vkClearColorValue.int32[0] = rhiClearValueElement.m_color.m_int32[0];
		vkClearColorValue.int32[1] = rhiClearValueElement.m_color.m_int32[1];
		vkClearColorValue.int32[2] = rhiClearValueElement.m_color.m_int32[2];
		vkClearColorValue.int32[3] = rhiClearValueElement.m_color.m_int32[3];
		vkClearColorValue.uint32[0] = rhiClearValueElement.m_color.m_uint32[0];
		vkClearColorValue.uint32[1] = rhiClearValueElement.m_color.m_uint32[1];
		vkClearColorValue.uint32[2] = rhiClearValueElement.m_color.m_uint32[2];
		vkClearColorValue.uint32[3] = rhiClearValueElement.m_color.m_uint32[3];

		VkClearDepthStencilValue vkClearDepthStencilValue;
		vkClearDepthStencilValue.depth = rhiClearValueElement.m_depthStencil.m_depth;
		vkClearDepthStencilValue.stencil = rhiClearValueElement.m_depthStencil.m_stencil;

		vkClearValueElement.color = vkClearColorValue;
		vkClearValueElement.depthStencil = vkClearDepthStencilValue;

	};

	VkRenderPassBeginInfo vkRenderPassBeginInfo{};
	vkRenderPassBeginInfo.sType = (VkStructureType)pRenderPassBegin->m_sType;
	vkRenderPassBeginInfo.pNext = pRenderPassBegin->m_pNext;
	vkRenderPassBeginInfo.renderPass = ((VulkanRenderPass*)pRenderPassBegin->m_pRenderPass)->GetResource();
	vkRenderPassBeginInfo.framebuffer = ((VulkanFramebuffer*)pRenderPassBegin->m_pFramebuffer)->GetResource();
	vkRenderPassBeginInfo.renderArea = rect2d;
	vkRenderPassBeginInfo.clearValueCount = pRenderPassBegin->m_clearValueCount;
	vkRenderPassBeginInfo.pClearValues = vkClearValueList.data();

	return _vkCmdBeginRenderPass(((VulkanCommandBuffer*)commandBuffer)->GetResource(), &vkRenderPassBeginInfo, (VkSubpassContents)contents);
}

void VulkanRHI::CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents)
{
	// 用于在渲染通道内切换到下一个子通道的命令。它只能在渲染通道内部调用，并且只能在命令缓冲区录制期间使用。
	// 实现渲染通道优化（如延迟渲染、Tile-Based Rendering）的关键机制，允许在单个渲染通道内组织多个渲染阶段，避免昂贵的渲染目标回写内存操作。
	return _vkCmdNextSubpass(((VulkanCommandBuffer*)commandBuffer)->GetResource(), ((VkSubpassContents)contents));
}

void VulkanRHI::CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer)
{
	// 用于结束当前渲染通道的命令。它标志着渲染通道的结束，与 vkCmdBeginRenderPass 成对使用，形成一个完整的渲染作用域。
	return _vkCmdEndRenderPass(((VulkanCommandBuffer*)commandBuffer)->GetResource());
}

void VulkanRHI::CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline)
{
	// 使用哪一个管线状态
	return _vkCmdBindPipeline(((VulkanCommandBuffer*)commandBuffer)->GetResource(), (VkPipelineBindPoint)pipelineBindPoint, ((VulkanPipeline*)pipeline)->GetResource());
}

void VulkanRHI::CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports)
{
	//viewport
	int viewportSize = viewportCount;
	std::vector<VkViewport> vkViewportList(viewportSize);
	for (int i = 0; i < viewportSize; ++i)
	{
		const auto& rhiViewportElement = pViewports[i];
		auto& vkViewportElement = vkViewportList[i];

		vkViewportElement.x = rhiViewportElement.m_x;
		vkViewportElement.y = rhiViewportElement.m_y;
		vkViewportElement.width = rhiViewportElement.m_width;
		vkViewportElement.height = rhiViewportElement.m_height;
		vkViewportElement.minDepth = rhiViewportElement.m_minDepth;
		vkViewportElement.maxDepth = rhiViewportElement.m_maxDepth;
	};

	// 必须先在管线创建时启用动态状态
	// 告诉 GPU："渲染出来的图像应该映射到帧缓冲区的哪个矩形区域"。
	return _vkCmdSetViewport(((VulkanCommandBuffer*)commandBuffer)->GetResource(), firstViewport, viewportCount, vkViewportList.data());
}

void VulkanRHI::CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors)
{
	//rect_2d
	int rect2dSize = scissorCount;
	std::vector<VkRect2D> vkRect2dList(rect2dSize);
	for (int i = 0; i < rect2dSize; ++i)
	{
		const auto& rhiRect2dElement = pScissors[i];
		auto& vkRect2dElement = vkRect2dList[i];

		VkOffset2D offset2d{};
		offset2d.x = rhiRect2dElement.m_offset.m_x;
		offset2d.y = rhiRect2dElement.m_offset.m_y;

		VkExtent2D extent2d{};
		extent2d.width = rhiRect2dElement.m_extent.m_width;
		extent2d.height = rhiRect2dElement.m_extent.m_height;

		vkRect2dElement.offset = (VkOffset2D)offset2d;
		vkRect2dElement.extent = (VkExtent2D)extent2d;

	};
	// 必须先在管线创建时启用动态状态
	// 设置图形管线的裁剪矩形，限制像素的最终输出区域。
	return _vkCmdSetScissor(((VulkanCommandBuffer*)commandBuffer)->GetResource(), firstScissor, scissorCount, vkRect2dList.data());
}

void VulkanRHI::CmdBindVertexBuffersPFN(RHICommandBuffer* commandBuffer, uint32_t firstBinding, uint32_t bindingCount, RHIBuffer* const* pBuffers, const RHIDeviceSize* pOffsets)
{        
	//buffer
	int bufferSize = bindingCount;
	std::vector<VkBuffer> vkBufferList(bufferSize);
	for (int i = 0; i < bufferSize; ++i)
	{
		const auto& rhiBufferElement = pBuffers[i];
		auto& vkBufferElement = vkBufferList[i];

		vkBufferElement = ((VulkanBuffer*)rhiBufferElement)->GetResource();
	};

	//offset
	int offsetSize = bindingCount;
	std::vector<VkDeviceSize> vkDeviceSizeList(offsetSize);
	for (int i = 0; i < offsetSize; ++i)
	{
		const auto& rhiOffsetElement = pOffsets[i];
		auto& vkOffsetElement = vkDeviceSizeList[i];

		vkOffsetElement = rhiOffsetElement;
	};

	// 顶点缓冲区绑定
	return _vkCmdBindVertexBuffers(((VulkanCommandBuffer*)commandBuffer)->GetResource(), firstBinding, bindingCount, vkBufferList.data(), vkDeviceSizeList.data());
}

void VulkanRHI::CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType)
{
	// 索引缓冲区绑定
	return _vkCmdBindIndexBuffer(((VulkanCommandBuffer*)commandBuffer)->GetResource(), ((VulkanBuffer*)buffer)->GetResource(), (VkDeviceSize)offset, (VkIndexType)indexType);
}

void VulkanRHI::CmdBindDescriptorSetsPFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipelineLayout* layout, uint32_t firstSet, uint32_t descriptorSetCount, const RHIDescriptorSet* const* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	//RHI转vk
	int descriptorSetSize = descriptorSetCount;
	std::vector<VkDescriptorSet> vkDescriptorSetList(descriptorSetSize);
	for (int i = 0; i < descriptorSetSize; ++i)
	{
		const auto& rhiDescriptorSetElement = pDescriptorSets[i];
		auto& vkDescriptorSetElement = vkDescriptorSetList[i];

		vkDescriptorSetElement = ((VulkanDescriptorSet*)rhiDescriptorSetElement)->GetResource();
	};

	//offset
	int offsetSize = dynamicOffsetCount;
	std::vector<uint32_t> vkOffsetList(offsetSize);
	for (int i = 0; i < offsetSize; ++i)
	{
		const auto& rhiOffsetElement = pDynamicOffsets[i];
		auto& vkOffsetElement = vkOffsetList[i];

		vkOffsetElement = rhiOffsetElement;
	};

	// 绑定创建好的描述符集
	return _vkCmdBindDescriptorSets(
		((VulkanCommandBuffer*)commandBuffer)->GetResource(),
		(VkPipelineBindPoint)pipelineBindPoint,
		((VulkanPipelineLayout*)layout)->GetResource(),
		firstSet,
		descriptorSetCount,
		vkDescriptorSetList.data(),
		dynamicOffsetCount,
		vkOffsetList.data());
}

void VulkanRHI::CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	// 实例化绘制
	return _vkCmdDrawIndexed(((VulkanCommandBuffer*)commandBuffer)->GetResource(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRHI::CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects)
{
	//clear_attachment
	int clearAttachmentSize = attachmentCount;
	std::vector<VkClearAttachment> vkClearAttachmentList(clearAttachmentSize);
	for (int i = 0; i < clearAttachmentSize; ++i)
	{
		const auto& rhiClearAttachmentElement = pAttachments[i];
		auto& vkClearAttachmentElement = vkClearAttachmentList[i];

		VkClearColorValue vkClearColorValue;
		vkClearColorValue.float32[0] = rhiClearAttachmentElement.m_clearValue.m_color.m_float32[0];
		vkClearColorValue.float32[1] = rhiClearAttachmentElement.m_clearValue.m_color.m_float32[1];
		vkClearColorValue.float32[2] = rhiClearAttachmentElement.m_clearValue.m_color.m_float32[2];
		vkClearColorValue.float32[3] = rhiClearAttachmentElement.m_clearValue.m_color.m_float32[3];
		vkClearColorValue.int32[0] = rhiClearAttachmentElement.m_clearValue.m_color.m_int32[0];
		vkClearColorValue.int32[1] = rhiClearAttachmentElement.m_clearValue.m_color.m_int32[1];
		vkClearColorValue.int32[2] = rhiClearAttachmentElement.m_clearValue.m_color.m_int32[2];
		vkClearColorValue.int32[3] = rhiClearAttachmentElement.m_clearValue.m_color.m_int32[3];
		vkClearColorValue.uint32[0] = rhiClearAttachmentElement.m_clearValue.m_color.m_uint32[0];
		vkClearColorValue.uint32[1] = rhiClearAttachmentElement.m_clearValue.m_color.m_uint32[1];
		vkClearColorValue.uint32[2] = rhiClearAttachmentElement.m_clearValue.m_color.m_uint32[2];
		vkClearColorValue.uint32[3] = rhiClearAttachmentElement.m_clearValue.m_color.m_uint32[3];

		VkClearDepthStencilValue vkClearDepthStencilValue;
		vkClearDepthStencilValue.depth = rhiClearAttachmentElement.m_clearValue.m_depthStencil.m_depth;
		vkClearDepthStencilValue.stencil = rhiClearAttachmentElement.m_clearValue.m_depthStencil.m_stencil;

		vkClearAttachmentElement.clearValue.color = vkClearColorValue;
		vkClearAttachmentElement.clearValue.depthStencil = vkClearDepthStencilValue;
		vkClearAttachmentElement.aspectMask = rhiClearAttachmentElement.m_aspectMask;
		vkClearAttachmentElement.colorAttachment = rhiClearAttachmentElement.m_colorAttachment;
	};

	//clear_rect
	int clearRectSize = rectCount;
	std::vector<VkClearRect> vkClearRectList(clearRectSize);
	for (int i = 0; i < clearRectSize; ++i)
	{
		const auto& rhiClearRectElement = pRects[i];
		auto& vkClearRectElement = vkClearRectList[i];

		VkOffset2D offset2d{};
		offset2d.x = rhiClearRectElement.m_rect.m_offset.m_x;
		offset2d.y = rhiClearRectElement.m_rect.m_offset.m_y;

		VkExtent2D extent2d{};
		extent2d.width = rhiClearRectElement.m_rect.m_extent.m_width;
		extent2d.height = rhiClearRectElement.m_rect.m_extent.m_height;

		vkClearRectElement.rect.offset = (VkOffset2D)offset2d;
		vkClearRectElement.rect.extent = (VkExtent2D)extent2d;
		vkClearRectElement.baseArrayLayer = rhiClearRectElement.m_baseArrayLayer;
		vkClearRectElement.layerCount = rhiClearRectElement.m_layerCount;
	};

	return _vkCmdClearAttachments(((VulkanCommandBuffer*)commandBuffer)->GetResource(), attachmentCount, vkClearAttachmentList.data(), rectCount, vkClearRectList.data());
}

bool VulkanRHI::BeginCommandBuffer(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo)
{
	return false;
}

void VulkanRHI::CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const ST_RHIBufferImageCopy* pRegions)
{
}

void VulkanRHI::CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height)
{
}

void VulkanRHI::CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, ST_RHIBufferCopy* pRegions)
{
}

void VulkanRHI::CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
}

void VulkanRHI::CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
}

void VulkanRHI::CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset)
{
}

void VulkanRHI::CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const ST_RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ST_RHIImageMemoryBarrier* pImageMemoryBarriers)
{
}

bool VulkanRHI::EndCommandBuffer(RHICommandBuffer* commandBuffer)
{
	return false;
}

void VulkanRHI::UpdateDescriptorSets(uint32_t descriptorWriteCount, const ST_RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const ST_RHICopyDescriptorSet* pDescriptorCopies)
{
	int writeDescriptorSetSize = descriptorWriteCount;	// 写入描述符集数量
	std::vector<VkWriteDescriptorSet> vkWriteDescriptorSetList(writeDescriptorSetSize);

	// 计算 imageInfo 和 bufferInfo 的数量
	int imageInfoCount = 0;
	int bufferInfoCount = 0;
	for (int i = 0; i < writeDescriptorSetSize; ++i)
	{
		const auto& rhiWriteDescriptorSetElement = pDescriptorWrites[i];
		if (rhiWriteDescriptorSetElement.m_pImageInfo != nullptr)
		{
			imageInfoCount++;
		}
		if (rhiWriteDescriptorSetElement.m_pBufferInfo != nullptr)
		{
			bufferInfoCount++;
		}
	}

	std::vector<VkDescriptorImageInfo> vkDescriptorImageInfoList(imageInfoCount);
	std::vector<VkDescriptorBufferInfo> vkDescriptorBufferInfoList(bufferInfoCount);
	int imageInfoCurrent = 0;
	int bufferInfoCurrent = 0;

	// 填充 VkWriteDescriptorSet 结构体
	for (int i = 0; i < writeDescriptorSetSize; ++i)
	{
		const auto& rhiWriteDescriptorSetElement = pDescriptorWrites[i];
		auto& vkWriteDescriptorSetElement = vkWriteDescriptorSetList[i];

		const VkDescriptorImageInfo* vkDescriptorImageInfoPtr = nullptr;
		if (rhiWriteDescriptorSetElement.m_pImageInfo != nullptr)
		{
			auto& vkDescriptorImageInfo = vkDescriptorImageInfoList[imageInfoCurrent];
			if (rhiWriteDescriptorSetElement.m_pImageInfo->m_pSampler == nullptr)
			{
				vkDescriptorImageInfo.sampler = nullptr;
			}
			else
			{
				vkDescriptorImageInfo.sampler = ((VulkanSampler*)rhiWriteDescriptorSetElement.m_pImageInfo->m_pSampler)->GetResource();
			}
			vkDescriptorImageInfo.imageView = ((VulkanImageView*)rhiWriteDescriptorSetElement.m_pImageInfo->m_pImageView)->GetResource();
			vkDescriptorImageInfo.imageLayout = (VkImageLayout)rhiWriteDescriptorSetElement.m_pImageInfo->m_imageLayout;

			vkDescriptorImageInfoPtr = &vkDescriptorImageInfo;
			imageInfoCurrent++;
		}

		const VkDescriptorBufferInfo* vkDescriptorBufferInfoPtr = nullptr;
		if (rhiWriteDescriptorSetElement.m_pBufferInfo != nullptr)
		{
			auto& vkDescriptorBufferInfo = vkDescriptorBufferInfoList[bufferInfoCurrent];
			vkDescriptorBufferInfo.buffer = ((VulkanBuffer*)rhiWriteDescriptorSetElement.m_pBufferInfo->m_pBuffer)->GetResource();
			vkDescriptorBufferInfo.offset = (VkDeviceSize)rhiWriteDescriptorSetElement.m_pBufferInfo->m_offset;
			vkDescriptorBufferInfo.range = (VkDeviceSize)rhiWriteDescriptorSetElement.m_pBufferInfo->m_range;

			vkDescriptorBufferInfoPtr = &vkDescriptorBufferInfo;
			bufferInfoCurrent++;
		}

		vkWriteDescriptorSetElement.sType = (VkStructureType)rhiWriteDescriptorSetElement.m_sType;
		vkWriteDescriptorSetElement.pNext = (const void*)rhiWriteDescriptorSetElement.m_pNext;
		vkWriteDescriptorSetElement.dstSet = ((VulkanDescriptorSet*)rhiWriteDescriptorSetElement.m_pDstSet)->GetResource();
		vkWriteDescriptorSetElement.dstBinding = rhiWriteDescriptorSetElement.m_dstBinding;
		vkWriteDescriptorSetElement.dstArrayElement = rhiWriteDescriptorSetElement.m_dstArrayElement;
		vkWriteDescriptorSetElement.descriptorCount = rhiWriteDescriptorSetElement.m_descriptorCount;
		vkWriteDescriptorSetElement.descriptorType = (VkDescriptorType)rhiWriteDescriptorSetElement.m_descriptorType;
		vkWriteDescriptorSetElement.pImageInfo = vkDescriptorImageInfoPtr;
		vkWriteDescriptorSetElement.pBufferInfo = vkDescriptorBufferInfoPtr;
		//vk_write_descriptor_set_element.pTexelBufferView = &((VulkanBufferView*)rhi_write_descriptor_set_element.pTexelBufferView)->getResource();
	};

	// 检查 imageInfo 和 bufferInfo 的数量是否正确
	if (imageInfoCurrent != imageInfoCount || bufferInfoCurrent != bufferInfoCount)
	{
		LOG_ERROR("image_info_current != image_info_count || buffer_info_current != buffer_info_count");
		return;
	}

	// 填充 VkCopyDescriptorSet 结构体
	int copyDescriptorSetSize = descriptorCopyCount;
	std::vector<VkCopyDescriptorSet> vkCopyDescriptorSetList(copyDescriptorSetSize);
	for (int i = 0; i < copyDescriptorSetSize; ++i)
	{
		const auto& rhiCopyDescriptorSetElement = pDescriptorCopies[i];
		auto& vkCopyDescriptorSetElement = vkCopyDescriptorSetList[i];

		vkCopyDescriptorSetElement.sType = (VkStructureType)rhiCopyDescriptorSetElement.m_sType;
		vkCopyDescriptorSetElement.pNext = (const void*)rhiCopyDescriptorSetElement.m_pNext;
		vkCopyDescriptorSetElement.srcSet = ((VulkanDescriptorSet*)rhiCopyDescriptorSetElement.m_pSrcSet)->GetResource();
		vkCopyDescriptorSetElement.srcBinding = rhiCopyDescriptorSetElement.m_srcBinding;
		vkCopyDescriptorSetElement.srcArrayElement = rhiCopyDescriptorSetElement.m_srcArrayElement;
		vkCopyDescriptorSetElement.dstSet = ((VulkanDescriptorSet*)rhiCopyDescriptorSetElement.m_pDstSet)->GetResource();
		vkCopyDescriptorSetElement.dstBinding = rhiCopyDescriptorSetElement.m_dstBinding;
		vkCopyDescriptorSetElement.dstArrayElement = rhiCopyDescriptorSetElement.m_dstArrayElement;
		vkCopyDescriptorSetElement.descriptorCount = rhiCopyDescriptorSetElement.m_descriptorCount;
	};

	// 更新描述符集
	vkUpdateDescriptorSets(m_device, descriptorWriteCount, vkWriteDescriptorSetList.data(), descriptorCopyCount, vkCopyDescriptorSetList.data());
}

bool VulkanRHI::QueueSubmit(RHIQueue* queue, uint32_t submitCount, const ST_RHISubmitInfo* pSubmits, RHIFence* fence)
{
	return false;
}

bool VulkanRHI::QueueWaitIdle(RHIQueue* queue)
{
	return false;
}

void VulkanRHI::ResetCommandPool()
{
	VkResult resResetCommandPool = _vkResetCommandPool(m_device, m_commandPools[m_currentFrameIndex], 0);	// 重置命令池
	if (VK_SUCCESS != resResetCommandPool)
	{
		LOG_ERROR("failed to synchronize");
	}
}

void VulkanRHI::WaitForFences()
{
	VkResult resWaitForFences = _vkWaitForFences(m_device, 1, &m_isFrameInFlightFences[m_currentFrameIndex], VK_TRUE, UINT64_MAX);
	if (VK_SUCCESS != resWaitForFences)
	{
		LOG_ERROR("failed to synchronize!");
	}
}

void VulkanRHI::GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties)
{
}

RHICommandBuffer* VulkanRHI::GetCurrentCommandBuffer() const
{
	return m_pCurrentCommandBuffer;
}

RHICommandBuffer* const* VulkanRHI::GetCommandBufferList() const
{
	return nullptr;
}

RHICommandPool* VulkanRHI::GetCommandPoor() const
{
	return nullptr;
}

RHIDescriptorPool* VulkanRHI::GetDescriptorPoor() const
{
	return m_rhiDescriptorPool;
}

RHIFence* const* VulkanRHI::GetFenceList() const
{
	return nullptr;
}

ST_QueueFamilyIndices VulkanRHI::GetQueueFamilyIndices() const
{
	return ST_QueueFamilyIndices();
}

RHIQueue* VulkanRHI::GetGraphicsQueue() const
{
	return nullptr;
}

RHIQueue* VulkanRHI::GetComputeQueue() const
{
	return nullptr;
}

ST_RHISwapChainDesc VulkanRHI::GetSwapchainInfo()
{
	ST_RHISwapChainDesc desc;
	desc.m_imageFormat = m_swapchainImageFormat;
	desc.m_extent = m_swapchainExtent;
	desc.m_pViewport = &m_viewport;
	desc.m_pScissor = &m_scissor;
	desc.m_imageViews = m_swapchainImageViews;
	return desc;
}

ST_RHIDepthImageDesc VulkanRHI::GetDepthImageInfo() const
{
	return ST_RHIDepthImageDesc();
}

uint8_t VulkanRHI::GetMaxFramesInFlight() const
{
	return 0;
}

uint8_t VulkanRHI::GetCurrentFrameIndex() const
{
	return m_currentFrameIndex;
}

void VulkanRHI::SetCurrentFrameIndex(uint8_t index)
{
}

RHICommandBuffer* VulkanRHI::BeginSingleTimeCommands()
{
	return nullptr;
}

void VulkanRHI::EndSingleTimeCommands(RHICommandBuffer* command_buffer)
{
}

bool VulkanRHI::PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain)
{
	// 简单来说，它的工作就是：“GPU，请给我下一张可以画的‘画布’的编号。”
	VkResult acquireImageResult = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableForRenderSemaphores[m_currentFrameIndex], VK_NULL_HANDLE, &m_currentSwapchainImageIndex);

	if (VK_ERROR_OUT_OF_DATE_KHR == acquireImageResult)	// 交换链已经过期，无法再用于渲染（通常发生在窗口调整大小后）。必须重新创建交换链后才能继续。
	{
		RecreateSwapChain();
		passUpdateAfterRecreateSwapchain();
	}
	else if (VK_SUBOPTIMAL_KHR == acquireImageResult)
	{
		// 获取到的图像是可用的，但交换链的表面属性（如窗口大小）不再与显示引擎完全匹配。应用程序可以继续使用这个图像进行渲染，但应该重新创建交换链以获得最佳性能。

		RecreateSwapChain();
		passUpdateAfterRecreateSwapchain();

		// NULL 提交等待信号量
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };	// 等待阶段
		VkSubmitInfo submitInfo = {};	// 提交信息
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;	// 等待信号量数量
		submitInfo.pWaitSemaphores = &m_imageAvailableForRenderSemaphores[m_currentFrameIndex];
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = NULL;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = NULL;

		// 变为 unsignaled
		VkResult resResetFences = _vkResetFences(m_device, 1, &m_isFrameInFlightFences[m_currentFrameIndex]);
		if (VK_SUCCESS != resResetFences)
		{
			LOG_ERROR("_vkResetFences failed!");
			return false;
		}

		VkResult resQueueSubmit = vkQueueSubmit(((VulkanQueue*)m_graphicsQueue)->GetResource(), 1, &submitInfo, m_isFrameInFlightFences[m_currentFrameIndex]);
		if (VK_SUCCESS != resQueueSubmit)
		{
			LOG_ERROR("vkQueueSubmit failed!");
			return false;
		}

		m_currentFrameIndex = (m_currentFrameIndex + 1) % s_maxFramesInFlight;
		return RHI_SUCCESS;
	}
	else
	{
		if (VK_SUCCESS != acquireImageResult)
		{
			LOG_ERROR("vkAcquireNextImageKHR failed!");
			return false;
		}
	}

	// begin command buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo{};	// 命令缓冲区开始信息
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;	// 继承信息

	// 开始记录命令
	VkResult resBeginCommandBuffer = _vkBeginCommandBuffer(m_vkCommandBuffers[m_currentFrameIndex], &commandBufferBeginInfo);
	if (VK_SUCCESS != resBeginCommandBuffer)
	{
		LOG_ERROR("_vkBeginCommandBuffer failed!");
		return false;
	}

	return false;
}

void VulkanRHI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
{
	// 结束命令缓冲区的记录
	VkResult resEndCommandBuffer = _vkEndCommandBuffer(m_vkCommandBuffers[m_currentFrameIndex]);
	if (VK_SUCCESS != resEndCommandBuffer)
	{
		LOG_ERROR("_vkEndCommandBuffer failed!");
		return;
	}

	VkSemaphore semaphores[2] = {
		((VulkanSemaphore*)m_imageAvailableForTexturescopySemaphores[m_currentFrameIndex])->GetResource(),
		m_imageFinishedForPresentationSemaphores[m_currentFrameIndex]
	};

	// 提交信息
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_imageAvailableForRenderSemaphores[m_currentFrameIndex];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkCommandBuffers[m_currentFrameIndex];
	submitInfo.signalSemaphoreCount = 2;
	submitInfo.pSignalSemaphores = semaphores;

	VkResult resResetFences = _vkResetFences(m_device, 1, &m_isFrameInFlightFences[m_currentFrameIndex]);
	if (VK_SUCCESS != resResetFences)
	{
		LOG_ERROR("_vkResetFences failed!");
		return;
	}

	VkResult resQueueSubmit = vkQueueSubmit(((VulkanQueue*)m_graphicsQueue)->GetResource(), 1, &submitInfo, m_isFrameInFlightFences[m_currentFrameIndex]);
	if (VK_SUCCESS != resQueueSubmit)
	{
		LOG_ERROR("vkQueueSubmit failed!");
		return;
	}

	// 提交呈现请求
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_imageFinishedForPresentationSemaphores[m_currentFrameIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &m_currentSwapchainImageIndex;

	VkResult presentResult = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if (VK_ERROR_OUT_OF_DATE_KHR == presentResult || VK_SUBOPTIMAL_KHR == presentResult)
	{
		RecreateSwapChain();
		passUpdateAfterRecreateSwapchain();
	}
	else
	{
		if (VK_SUCCESS != presentResult)
		{
			LOG_ERROR("vkQueuePresentKHR failed!");
			return;
		}
	}

	m_currentFrameIndex = (m_currentFrameIndex + 1) % s_maxFramesInFlight;
}

void VulkanRHI::PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color)
{
	if (m_enableDebugUtilsLabel)	// 如果启用调试工具标签
	{
		VkDebugUtilsLabelEXT labelInfo;
		labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		labelInfo.pNext = nullptr;
		labelInfo.pLabelName = name;
		for (int i = 0; i < 4; ++i)
		{
			labelInfo.color[i] = color[i];
		}
		// 用于调试和性能分析的命令。它的核心功能是在命令缓冲区中开始一个带标签的区域。(用于图形调试和性能分析)
		_vkCmdBeginDebugUtilsLabelEXT(((VulkanCommandBuffer*)commond_buffer)->GetResource(), &labelInfo);
	}
}

void VulkanRHI::PopEvent(RHICommandBuffer* commond_buffer)
{
	if (m_enableDebugUtilsLabel)
	{
		// 用于结束一个调试标签区域。
		_vkCmdEndDebugUtilsLabelEXT(((VulkanCommandBuffer*)commond_buffer)->GetResource());
	}
}

VulkanRHI::~VulkanRHI()
{
}

void VulkanRHI::Clear()
{
}

void VulkanRHI::ClearSwapchain()
{
}

void VulkanRHI::DestroyDefaultSampler(ERHIDefaultSamplerType type)
{
}

void VulkanRHI::DestroyMipmappedSampler()
{
}

void VulkanRHI::DestroyShaderModule(RHIShader* shaderModule)
{
	vkDestroyShaderModule(m_device, ((VulkanShader*)shaderModule)->GetResource(), nullptr);

	delete(shaderModule);
}

void VulkanRHI::DestroySemaphore(RHISemaphore* semaphore)
{
}

void VulkanRHI::DestroySampler(RHISampler* sampler)
{
}

void VulkanRHI::DestroyInstance(RHIInstance* instance)
{
}

void VulkanRHI::DestroyImageView(RHIImageView* imageView)
{
}

void VulkanRHI::DestroyImage(RHIImage* image)
{
}

void VulkanRHI::DestroyFramebuffer(RHIFramebuffer* framebuffer)
{
}

void VulkanRHI::DestroyFence(RHIFence* fence)
{
}

void VulkanRHI::DestroyDevice()
{
}

void VulkanRHI::DestroyCommandPool(RHICommandPool* commandPool)
{
}

void VulkanRHI::DestroyBuffer(RHIBuffer*& buffer)
{
}

void VulkanRHI::FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers)
{
}

void VulkanRHI::FreeMemory(RHIDeviceMemory*& memory)
{
}

bool VulkanRHI::MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData)
{
	return false;
}

void VulkanRHI::UnmapMemory(RHIDeviceMemory* memory)
{
}

void VulkanRHI::InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size)
{
}

void VulkanRHI::FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size)
{
}

RHISemaphore*& VulkanRHI::GetTextureCopySemaphore(uint32_t index)
{
	// TODO: 在此处插入 return 语句
	RHISemaphore* semaphore = nullptr;
	return semaphore;
}

void VulkanRHI::CreateInstance()
{
	if (m_enableValidationLayers && !CheckValidationLayerSupport())	// 检查验证层支持
	{
		LOG_ERROR("validation layers requested, but not available!");
	}

	m_vulkanApiVersion = VK_API_VERSION_1_0; // 设置Vulkan API版本

	VkApplicationInfo appInfo = {};	// 创建应用程序信息结构体
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;	// 设置结构体类型
	appInfo.pApplicationName = "XYH Engine Renderer";	// 设置应用程序名称
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	// 设置应用程序版本
	appInfo.pEngineName = "XYH Engine";	// 设置引擎名称
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);	// 设置引擎版本
	appInfo.apiVersion = m_vulkanApiVersion;	// 设置Vulkan API版本

	VkInstanceCreateInfo instanceCreateInfo = {};	// 创建实例创建信息结构体
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// 设置结构体类型
	instanceCreateInfo.pApplicationInfo = &appInfo;	// 设置应用程序信息

	instanceCreateInfo.enabledExtensionCount = 0;	// 禁用扩展
	instanceCreateInfo.ppEnabledExtensionNames = nullptr;	// 禁用扩展名称
	std::vector<const char*> requiredExtensions = GetRequiredExtensions();	// 获取所需的扩展列表
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());	// 设置所需的扩展数量
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();	// 设置所需的扩展名称

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};	// 创建调试消息处理器创建信息结构体
	if (m_enableValidationLayers || m_enableDebugUtilsLabel)	// 如果启用验证层或调试工具标签
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());	// 设置验证层数量
		instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();	// 设置验证层名称

		PopulateDebugMessengerCreateInfo(debugCreateInfo);	// 填充调试消息处理器创建信息
		instanceCreateInfo.pNext = &debugCreateInfo;	// 设置调试消息处理器创建信息
	}
	else
	{
		instanceCreateInfo.enabledLayerCount = 0;	// 禁用验证层
		instanceCreateInfo.pNext = nullptr;	// 不使用调试消息处理器
	}

	// 创建Vulkan实例
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create Vulkan instance!");	// 如果创建失败，输出错误信息
	}
}

void VulkanRHI::InitializeDebugMessenger()
{
	if (m_enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;	// 创建调试消息处理器创建信息结构体
		PopulateDebugMessengerCreateInfo(createInfo);	// 填充调试消息处理器创建信息
		if (VK_SUCCESS != CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger))	// 创建调试消息处理器
		{
			LOG_ERROR("failed to set up debug messenger!");	// 如果创建失败，输出错误信息
		}
	}

	if (m_enableDebugUtilsLabel)	// 如果启用调试工具标签
	{
		_vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT");	// 获取vkCmdBeginDebugUtilsLabelEXT函数指针
		_vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT");	// 获取vkCmdEndDebugUtilsLabelEXT函数指针
	}
}

void VulkanRHI::CreateWindowSurface()
{
	// 创建Vulkan表面
	if (glfwCreateWindowSurface(m_instance, m_pGLFWwindow, nullptr, &m_surface) != VK_SUCCESS)
	{
		LOG_ERROR("glfwCreateWindowSurface failed!");
	}
}

void VulkanRHI::InitializePhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;	// 获取物理设备数量
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);	// 获取物理设备数量
	if (physicalDeviceCount == 0)	// 如果没有可用的物理设备
	{
		LOG_ERROR("failed to find GPUs with Vulkan support!");	// 输出错误信息
	}
	else
	{
		//找到一个符合我们要求的设备
		//或者找到最好的
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);	// 创建一个向量来存储物理设备
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());	// 获取物理设备列表
		std::vector<std::pair<int, VkPhysicalDevice>> rankedPhysicalDevices;	// 创建一个向量来存储物理设备和其评分
		for (const auto& device : physicalDevices)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;	// 创建物理设备属性结构体
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);	// 获取物理设备属性

			int score = 0;	// 初始化评分为0
			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // 独立显卡
			{
				score += 1000;
			}
			else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)	// 集成显卡
			{
				score += 100;
			}

			rankedPhysicalDevices.push_back({ score, device });
		}

		// 显卡评分排序
		std::sort(rankedPhysicalDevices.begin(),
			rankedPhysicalDevices.end(),
			[](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
				return p1 > p2;
			});

		// 遍历排序后的物理设备列表，选择第一个符合要求的设备
		for (const auto& device : rankedPhysicalDevices)
		{
			if (IsDeviceSuitable(device.second))
			{
				m_physicalDevice = device.second;
				break;
			}
		}
		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			LOG_ERROR("failed to find suitable physical device");
		}
	}
}

void VulkanRHI::CreateLogicalDevice()
{
	m_queueIndices = FindQueueFamilies(m_physicalDevice);	// 查找物理设备的队列族

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;	// 队列创建信息
	std::set<uint32_t> queueFamilies = { m_queueIndices.m_graphicsFamily.value(), m_queueIndices.m_presentFamily.value(), m_queueIndices.m_computeFamily.value() };

	float queuePriority = 1.0f;	// 队列优先级
	for (uint32_t queueFamily : queueFamilies)
	{
		// 队列创建信息
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;	// 设置结构体类型
		queueCreateInfo.queueFamilyIndex = queueFamily;	// 设置队列族索引
		queueCreateInfo.queueCount = 1;	// 设置队列数量
		queueCreateInfo.pQueuePriorities = &queuePriority;	// 设置队列优先级
		queueCreateInfos.push_back(queueCreateInfo);	// 将队列创建信息添加到向量中
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};	// 物理设备特性
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;	// 启用各向异性过滤
	physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;	// 启用片段着色器存储和原子操作
	physicalDeviceFeatures.independentBlend = VK_TRUE;	// 启用独立混合
	if (m_enablePointLightShadow)
	{
		physicalDeviceFeatures.geometryShader = VK_TRUE;	// 启用几何着色器
	}

	VkDeviceCreateInfo deviceCreateInfo = {};	// 创建逻辑设备信息
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;	// 设置结构体类型
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());	// 设置队列创建信息数量
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();	// 设置队列创建信息
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;	// 设置物理设备特性
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());	// 设置设备扩展数量
	deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();	// 设置设备扩展名称
	deviceCreateInfo.enabledLayerCount = 0;	// 启用设备层（Device Layers），用于调试或验证（现代 Vulkan 已弃用，推荐使用实例层）。

	// 创建逻辑设备
	if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create logical device!");	// 如果创建失败，输出错误信息
	}

	VkQueue vkGraphicsQueue;	// 图形队列
	vkGetDeviceQueue(m_device, m_queueIndices.m_graphicsFamily.value(), 0, &vkGraphicsQueue);	// 获取图形队列
	m_graphicsQueue = new VulkanQueue();	// 创建图形队列对象
	((VulkanQueue*)m_graphicsQueue)->SetResource(vkGraphicsQueue);	// 设置图形队列资源

	vkGetDeviceQueue(m_device, m_queueIndices.m_presentFamily.value(), 0, &m_presentQueue);	// 获取呈现队列

	VkQueue vkComputeQueue;	// 计算队列
	vkGetDeviceQueue(m_device, m_queueIndices.m_computeFamily.value(), 0, &vkComputeQueue);	// 获取计算队列
	m_computeQueue = new VulkanQueue();	// 创建计算队列对象
	((VulkanQueue*)m_computeQueue)->SetResource(vkComputeQueue);	// 设置计算队列资源

	// 获取设备函数指针
	_vkResetCommandPool = (PFN_vkResetCommandPool)vkGetDeviceProcAddr(m_device, "vkResetCommandPool");
	_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(m_device, "vkBeginCommandBuffer");
	_vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(m_device, "vkEndCommandBuffer");
	_vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetDeviceProcAddr(m_device, "vkCmdBeginRenderPass");
	_vkCmdNextSubpass = (PFN_vkCmdNextSubpass)vkGetDeviceProcAddr(m_device, "vkCmdNextSubpass");
	_vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)vkGetDeviceProcAddr(m_device, "vkCmdEndRenderPass");
	_vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(m_device, "vkCmdBindPipeline");
	_vkCmdSetViewport = (PFN_vkCmdSetViewport)vkGetDeviceProcAddr(m_device, "vkCmdSetViewport");
	_vkCmdSetScissor = (PFN_vkCmdSetScissor)vkGetDeviceProcAddr(m_device, "vkCmdSetScissor");
	_vkWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(m_device, "vkWaitForFences");
	_vkResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(m_device, "vkResetFences");
	_vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(m_device, "vkCmdDrawIndexed");
	_vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(m_device, "vkCmdBindVertexBuffers");
	_vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(m_device, "vkCmdBindIndexBuffer");
	_vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(m_device, "vkCmdBindDescriptorSets");
	_vkCmdClearAttachments = (PFN_vkCmdClearAttachments)vkGetDeviceProcAddr(m_device, "vkCmdClearAttachments");

	m_depthImageFormat = (ERHIFormat)FindDepthFormat();	// 查找深度格式

	if (1)
	{

	}
}

void VulkanRHI::CreateCommandPool()
{
	// 默认命令池
	{
		m_rhiCommandPool = new VulkanCommandPool();
		VkCommandPool vkCommandPool;
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};	// 命令池创建信息
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;	// 设置结构体类型
		commandPoolCreateInfo.pNext = nullptr;	// 不使用扩展
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	// 设置命令池标志位，允许重置命令缓冲区
		commandPoolCreateInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily.value();	// 设置队列族索引
		if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &vkCommandPool) != VK_SUCCESS)
		{
			LOG_ERROR("vk create command pool");
		}
		((VulkanCommandPool*)m_rhiCommandPool)->SetResource(vkCommandPool);	// 设置命令池资源
	}

	// 其他命令池
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = NULL;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;	// 设置命令池标志位，允许临时命令缓冲区
		commandPoolCreateInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily.value();
		// 创建多个命令池以支持多帧渲染
		for (uint32_t i = 0; i < s_maxFramesInFlight; ++i)
		{
			if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPools[i]) != VK_SUCCESS)
			{
				LOG_ERROR("vk create command pool");
			}
		}
	}
}

void VulkanRHI::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};	// 命令缓冲区分配信息
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;	// 设置结构体类型
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// 设置命令缓冲区级别为主级别
	commandBufferAllocateInfo.commandBufferCount = 1U;	// 设置命令缓冲区数量为1

	// 分配命令缓冲区
	for (uint32_t i = 0; i < s_maxFramesInFlight; ++i)
	{
		commandBufferAllocateInfo.commandPool = m_commandPools[i];
		VkCommandBuffer vkCommandBuffer;
		if (vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &vkCommandBuffer) != VK_SUCCESS)
		{
			LOG_ERROR("vk allocate command buffers");
		}
		m_vkCommandBuffers[i] = vkCommandBuffer;
		m_commandBuffers[i] = new VulkanCommandBuffer();
		((VulkanCommandBuffer*)m_commandBuffers[i])->SetResource(vkCommandBuffer);	// 设置命令缓冲区资源
	}
}

void VulkanRHI::CreateDescriptorPool()
{
	// 由于DescriptorSet在Vulkan中应该被视为资产，所以DescriptorPool应该
	// 足够大，这样我们就可以从DescriptorPool中分配DescriptorSet，就像我
	// 们从devicemmemory中分配Buffer / Image一样。

	VkDescriptorPoolSize poolSizes[7];	// 创建描述符池大小数组
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;	// 存储缓冲区动态描述符类型
	poolSizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;	// 描述符数量
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;	// 存储缓冲区描述符类型
	poolSizes[1].descriptorCount = 1 + 1 + 1 * m_maxVertexBlendingMeshCount;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// 统一缓冲区描述符类型
	poolSizes[2].descriptorCount = 1 * m_maxMaterialCount;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// 组合图像采样器描述符类型
	poolSizes[3].descriptorCount = 3 + 5 * m_maxMaterialCount + 1 + 1; // ImGui_ImplVulkan_CreateDeviceObjects
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;	// 输入附件描述符类型
	poolSizes[4].descriptorCount = 4 + 1 + 1 + 2;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// 统一缓冲区动态描述符类型
	poolSizes[5].descriptorCount = 3;
	poolSizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;	// 存储图像描述符类型
	poolSizes[6].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;	// 设置结构体类型
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1 + 1 + 1 + m_maxMaterialCount + m_maxVertexBlendingMeshCount + 1 + 1; // 设置最大描述符集数量
	poolInfo.flags = 0U;	// 描述符池标志位

	// 创建描述符池
	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create descriptor pool!");	// 如果创建失败，输出错误信息
	}

	m_rhiDescriptorPool = new VulkanDescriptorPool();	// 创建Vulkan描述符池对象
	((VulkanDescriptorPool*)m_rhiDescriptorPool)->SetResource(m_vkDescriptorPool);	// 设置Vulkan描述符池资源
}

// semaphore : signal an image is ready for rendering // ready for presentation
// (m_vulkan_context._swapchain_images --> semaphores, fences)
void VulkanRHI::CreateSyncPrimitives()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};	// 信号量创建信息
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};	// 围栏创建信息
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 栅栏被初始化为信号

	// 创建信号量和围栏
	for (uint32_t i = 0; i < s_maxFramesInFlight; i++)
	{
		m_imageAvailableForTexturescopySemaphores[i] = new VulkanSemaphore();
		if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableForRenderSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageFinishedForPresentationSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &(((VulkanSemaphore*)m_imageAvailableForTexturescopySemaphores[i])->GetResource())) != VK_SUCCESS ||
			vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_isFrameInFlightFences[i]) != VK_SUCCESS)
		{
			LOG_ERROR("vk create semaphore & fence");
		}

		m_rhiIsFrameInFlightFences[i] = new VulkanFence();
		((VulkanFence*)m_rhiIsFrameInFlightFences[i])->SetResource(m_isFrameInFlightFences[i]);
	}
}

void VulkanRHI::CreateAssetAllocator()
{
	VmaVulkanFunctions vulkanFunctions = {};	// VMA Vulkan函数指针结构体
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	// 设置VMA分配器创建信息
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = m_vulkanApiVersion;	// 设置Vulkan API版本
	allocatorCreateInfo.physicalDevice = m_physicalDevice;	// 设置物理设备
	allocatorCreateInfo.device = m_device;	// 设置逻辑设备
	allocatorCreateInfo.instance = m_instance;	// 设置Vulkan实例
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;	// 设置Vulkan函数指针

	// 创建VMA分配器
	vmaCreateAllocator(&allocatorCreateInfo, &m_assetsAllocator);
}

bool VulkanRHI::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);	// 获取可用的验证层数量

	std::vector<VkLayerProperties> availableLayers(layerCount);	// 创建一个向量来存储验证层属性
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());	// 获取验证层属性

	for (const char* layerName : m_validationLayers)
	{
		bool layerFound = false;	// 标志变量，表示是否找到验证层
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)	// 比较验证层名称
			{
				layerFound = true;	// 找到验证层
				break;
			}
		}

		if (!layerFound)
		{
			return false;	// 如果没有找到某个验证层，则返回false
		}
	}

	return RHI_SUCCESS;
}

std::vector<const char*> VulkanRHI::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;	// 获取GLFW所需的扩展数量
	const char** glfwExtensions;	// 获取GLFW所需的扩展名称
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);	// 获取GLFW所需的扩展名称

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (m_enableValidationLayers || m_enableDebugUtilsLabel)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

#if defined(__MACH__)
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

	return extensions;
}

// 调试回调函数，用于处理Vulkan的调试消息
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void VulkanRHI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkResult VulkanRHI::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");	// 获取vkCreateDebugUtilsMessengerEXT函数指针
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

bool VulkanRHI::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	auto queueIndices = FindQueueFamilies(physicalDevice);	// 查找设备的队列族
	bool isExtensionSupported = CheckDeviceExtensionSupport(physicalDevice);	// 检查设备是否支持所需的扩展
	bool isSwapChainAdequate = false;	// 初始化交换链是否足够
	if (isExtensionSupported)
	{
		ST_SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);	// 查询交换链支持情况
		isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();	// 如果交换链格式和呈现模式都不为空，则认为交换链足够
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures;	// 特性
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);	// 获取物理设备特性

	if (!queueIndices.isComplete() || !isSwapChainAdequate || !physicalDeviceFeatures.samplerAnisotropy)	// 如果队列族不完整、交换链不够或物理设备不支持各向异性过滤
	{
		return false;	// 返回false，表示设备不适合
	}

	return true;
}

ST_QueueFamilyIndices VulkanRHI::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	ST_QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0u;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);	// 获取队列族数量
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());	// 获取队列族属性

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) // 如果队列族支持图形操作
		{
			indices.m_graphicsFamily = i;	// 设置图形队列族索引
		}

		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)	// 如果队列族支持计算操作
		{
			indices.m_computeFamily = i;	// 设置计算队列族索引
		}

		VkBool32 presentSupport = false;	// 呈现支持
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);	// 获取物理设备表面支持情况
		if (presentSupport)
		{
			indices.m_presentFamily = i;	// 设置呈现队列族索引
		}

		if (indices.isComplete())	// 如果所有队列族索引都已设置
		{
			break;	// 退出循环
		}

		++i;
	}

	return indices;
}

bool VulkanRHI::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);	// 获取设备扩展数量
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);	// 可用的设备扩展
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());	// 获取设备扩展属性

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());	// 将所需的设备扩展转换为集合
	for (const auto& extension : availableExtensions)	// 遍历可用的设备扩展
	{
		requiredExtensions.erase(extension.extensionName);	// 从所需的设备扩展中删除已找到的扩展
	}

	return requiredExtensions.empty();	// 如果所需的设备扩展集合为空，则表示所有所需的设备扩展都已找到
}

ST_SwapChainSupportDetails VulkanRHI::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	ST_SwapChainSupportDetails detailsResult;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &detailsResult.capabilities);	// 获取交换链表面能力

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);	// 获取交换链表面格式数量
	if (formatCount != 0)
	{
		detailsResult.formats.resize(formatCount);	// 调整交换链表面格式向量大小
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, detailsResult.formats.data());	// 获取交换链表面格式
	}

	uint32_t presentModeCount;	// 获取交换链表面呈现模式数量
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);	// 获取交换链表面呈现模式数量
	if (presentModeCount != 0)
	{
		detailsResult.presentModes.resize(presentModeCount);	// 调整交换链表面呈现模式向量大小
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, detailsResult.presentModes.data());	// 获取交换链表面呈现模式
	}

	return detailsResult;
}

VkFormat VulkanRHI::FindDepthFormat()
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat VulkanRHI::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	// 遍历候选格式
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	LOG_ERROR("findSupportedFormat failed");
	return VkFormat();
}

VkSurfaceFormatKHR VulkanRHI::ChooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
{
	// 遍历可用的表面格式
	for (const auto& surfaceFormat : availableSurfaceFormats)
	{
		// TODO：选择VK_FORMAT_B8G8R8A8_SRGB表面格式
		// 不需要在片段着色器中做伽马校正
		// 选择VK_FORMAT_B8G8R8A8_UNORM表面格式
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return surfaceFormat;
		}
	}
	return availableSurfaceFormats[0];
}

VkPresentModeKHR VulkanRHI::ChooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (VkPresentModeKHR presentMode : availablePresentModes)
	{
		// 优先选择VK_PRESENT_MODE_MAILBOX_KHR（邮箱模式），它可以减少延迟
		if (VK_PRESENT_MODE_MAILBOX_KHR == presentMode)
		{
			return VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;	// 默认使用VK_PRESENT_MODE_FIFO_KHR（FIFO模式）
}

VkExtent2D VulkanRHI::ChooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// 如果当前扩展的宽度和高度不是UINT32_MAX，则返回当前扩展
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_pGLFWwindow, &width, &height);	// 获取窗口的帧缓冲区大小

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };	// 创建实际扩展

		// 确保实际扩展在最小和最大范围内
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

NAMESPACE_XYH_END


