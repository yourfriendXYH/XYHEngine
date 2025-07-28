#include "VulkanRHI.h"
#include "../../WindowSystem.h"
#include "../../../../Core/Macro.h"
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

}

void VulkanRHI::PrepareContext()
{
}

bool VulkanRHI::IsPointLightShadowEnabled()
{
	return false;
}

bool VulkanRHI::AllocateCommandBuffer()
{
	return false;
}

bool VulkanRHI::AllocateDescriptorSet()
{
	return false;
}

void VulkanRHI::CreateSwapChain()
{
}

void VulkanRHI::RecreateSwapChain()
{
}

void VulkanRHI::CreateSwapChainImageViews()
{
}

void VulkanRHI::CreateFramebufferImageAndViews()
{
}

RHISampler* VulkanRHI::GetOrCreateDefaultSampler(ERHIDefaultSamplerType type)
{
	return nullptr;
}

RHISampler* VulkanRHI::GetOrCreateMipmapSampler(uint32_t width, uint32_t height)
{
	return nullptr;
}

RHIShader* VulkanRHI::CreateShaderModule(const std::vector<unsigned char>& shaderCode)
{
	return nullptr;
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

void VulkanRHI::CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags, RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels)
{
}

void VulkanRHI::CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels, RHIImageView*& image_view)
{
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
	return false;
}

bool VulkanRHI::CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence)
{
	return false;
}

bool VulkanRHI::CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer)
{
	return false;
}

bool VulkanRHI::CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines)
{
	return false;
}

bool VulkanRHI::CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines)
{
	return false;
}

bool VulkanRHI::CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout)
{
	return false;
}

bool VulkanRHI::CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass)
{
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
}

void VulkanRHI::CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents)
{
}

void VulkanRHI::CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer)
{
}

void VulkanRHI::CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline)
{
}

void VulkanRHI::CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports)
{
}

void VulkanRHI::CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors)
{
}

void VulkanRHI::CmdBindVertexBuffersPFN(RHICommandBuffer* commandBuffer, uint32_t firstBinding, uint32_t bindingCount, RHIBuffer* const* pBuffers, const RHIDeviceSize* pOffsets)
{
}

void VulkanRHI::CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType)
{
}

void VulkanRHI::CmdBindDescriptorSetsPFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipelineLayout* layout, uint32_t firstSet, uint32_t descriptorSetCount, const RHIDescriptorSet* const* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
}

void VulkanRHI::CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
}

void VulkanRHI::CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects)
{
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
}

void VulkanRHI::WaitForFences()
{
}

void VulkanRHI::GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties)
{
}

RHICommandBuffer* VulkanRHI::GetCurrentCommandBuffer() const
{
	return nullptr;
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
	return nullptr;
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
	return ST_RHISwapChainDesc();
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
	return 0;
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
	return false;
}

void VulkanRHI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
{
}

void VulkanRHI::PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color)
{
}

void VulkanRHI::PopEvent(RHICommandBuffer* commond_buffer)
{
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

void VulkanRHI::DestroyShaderModule(RHIShader* shader)
{
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
	std::set<uint32_t> queueFamilies = { m_queueIndices.m_graphicsFamily.value(), m_queueIndices.m_presentFamily.value(), m_queueIndices.m_computeFamily.value()};

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


}

void VulkanRHI::CreateCommandPool()
{
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

NAMESPACE_XYH_END


