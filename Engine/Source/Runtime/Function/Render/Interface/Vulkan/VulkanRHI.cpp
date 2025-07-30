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
	m_pGLFWwindow = initInfo.m_pWindowSystem->GetWindow(); // ��ȡGLFW����ָ��
	std::array<int, 2> windowSize = initInfo.m_pWindowSystem->GetWindowSize(); // ��ȡ���ڴ�С
	m_viewport = { 0.0f, 0.0f, static_cast<float>(windowSize[0]), static_cast<float>(windowSize[1]), 0.0f, 1.0f }; // �����ӿڴ�С
	m_scissor = { { 0, 0 }, { (uint32_t)windowSize[0], (uint32_t)windowSize[1] } }; // ���òü������С

#ifndef NDEBUG	// debugģʽ��������֤��͵��Թ��߱�ǩ
	m_enableValidationLayers = true;
	m_enableDebugUtilsLabel = true;
#else
	m_enable_validation_Layers = false;
	m_enable_debug_utils_label = false;
#endif

#if defined(__GNUC__) && defined(__MACH__)	// macOSƽ̨�½��õ��Դ��Ӱ
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
#elif defined(_MSC_VER)	// Microsoft Visual Studioƽ̨������VK_LAYER_PATH��������
	// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
	char const* vk_layer_path = "D:\\DirectX12_learn\\XYHEngine\\Engine\\3rdparty\\VulkanSDK\\bin\\Win32";
	SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
	SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
#else
#error Unknown Compiler
#endif

	CreateInstance(); // ����Vulkanʵ��

	InitializeDebugMessenger(); // ��ʼ��������Ϣ������

	CreateWindowSurface(); // �������ڱ���

	InitializePhysicalDevice(); // ��ʼ�������豸

	CreateLogicalDevice();	// �����߼��豸

	CreateCommandPool();	// ���������

	CreateCommandBuffers();	// �����������

	CreateDescriptorPool();	// ������������

	CreateSyncPrimitives();	// ����ͬ��ԭ��

	CreateSwapChain();	// ����������

	CreateSwapChainImageViews();	// ����������ͼ����ͼ
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
	// ��ѯ�������豸������֧����
	ST_SwapChainSupportDetails swapchainSupportDetails = QuerySwapChainSupport(m_physicalDevice);

	// choose the best or fitting format
	VkSurfaceFormatKHR chosenSurfaceFormat = ChooseSwapchainSurfaceFormatFromDetails(swapchainSupportDetails.formats);

	// choose the best or fitting present mode
	VkPresentModeKHR chosenPresentMode = ChooseSwapchainPresentModeFromDetails(swapchainSupportDetails.presentModes);

	// ѡ������ʵ���Ļ��С
	VkExtent2D chosenExtent = ChooseSwapchainExtentFromDetails(swapchainSupportDetails.capabilities);

	uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
	if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount)
	{
		imageCount = swapchainSupportDetails.capabilities.maxImageCount;
	}

	// ������������������Ϣ
	// KHR	Khronos �ٷ���׼����չ���糧��֧�֡�
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;	// �����������ı���
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = chosenSurfaceFormat.format;
	createInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
	createInfo.imageExtent = chosenExtent;	// �������Ŀ�Ⱥ͸߶�
	createInfo.imageArrayLayers = 1;	// ͼ��Ĳ�����ͨ��Ϊ1
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// ͼ���ʹ�÷�ʽ��ͨ��Ϊ��ɫ����
	// ����������
	uint32_t queueFamilyIndices[] = { m_queueIndices.m_graphicsFamily.value(), m_queueIndices.m_presentFamily.value() };
	// ���ͼ�ζ�����ͳ��ֶ����岻ͬ
	if (m_queueIndices.m_graphicsFamily != m_queueIndices.m_presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	// ����ģʽΪ����
		createInfo.queueFamilyIndexCount = 2;	// ����������������
		createInfo.pQueueFamilyIndices = queueFamilyIndices;	// ��������������
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	// ����ģʽΪ��ռ
	}
	createInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;	// �������ĵ�ǰ�任
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// �ϳ�Alphaͨ���ķ�ʽ��ͨ��Ϊ��͸��
	createInfo.presentMode = chosenPresentMode;	// �������ĳ���ģʽ
	createInfo.clipped = VK_TRUE;	// �Ƿ�ü�������ͼ��
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// ����������
	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
	{
		LOG_ERROR("vk create swapchain khr");
	}

	// ��ȡ������ͼ��
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

	// ���ý���������
	m_swapchainImageFormat = (ERHIFormat)chosenSurfaceFormat.format;
	m_swapchainExtent.m_height = chosenExtent.height;
	m_swapchainExtent.m_width = chosenExtent.width;

	m_scissor = { {0, 0}, {m_swapchainExtent.m_width, m_swapchainExtent.m_height} };	// ���òü������С
}

void VulkanRHI::RecreateSwapChain()
{
}

void VulkanRHI::CreateSwapChainImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());

	// ����������ͼ�񣬴���ͼ����ͼ
	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageView vkImageView;
		vkImageView = VulkanUtil::CreateImageView(m_device, m_swapchainImages[i], (VkFormat)m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
		m_swapchainImageViews[i] = new VulkanImageView();
		((VulkanImageView*)m_swapchainImageViews[i])->SetResource(vkImageView);
	}
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
	// TODO: �ڴ˴����� return ���
	RHISemaphore* semaphore = nullptr;
	return semaphore;
}

void VulkanRHI::CreateInstance()
{
	if (m_enableValidationLayers && !CheckValidationLayerSupport())	// �����֤��֧��
	{
		LOG_ERROR("validation layers requested, but not available!");
	}

	m_vulkanApiVersion = VK_API_VERSION_1_0; // ����Vulkan API�汾

	VkApplicationInfo appInfo = {};	// ����Ӧ�ó�����Ϣ�ṹ��
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;	// ���ýṹ������
	appInfo.pApplicationName = "XYH Engine Renderer";	// ����Ӧ�ó�������
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	// ����Ӧ�ó���汾
	appInfo.pEngineName = "XYH Engine";	// ������������
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);	// ��������汾
	appInfo.apiVersion = m_vulkanApiVersion;	// ����Vulkan API�汾

	VkInstanceCreateInfo instanceCreateInfo = {};	// ����ʵ��������Ϣ�ṹ��
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// ���ýṹ������
	instanceCreateInfo.pApplicationInfo = &appInfo;	// ����Ӧ�ó�����Ϣ

	instanceCreateInfo.enabledExtensionCount = 0;	// ������չ
	instanceCreateInfo.ppEnabledExtensionNames = nullptr;	// ������չ����
	std::vector<const char*> requiredExtensions = GetRequiredExtensions();	// ��ȡ�������չ�б�
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());	// �����������չ����
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();	// �����������չ����

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};	// ����������Ϣ������������Ϣ�ṹ��
	if (m_enableValidationLayers || m_enableDebugUtilsLabel)	// ���������֤�����Թ��߱�ǩ
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());	// ������֤������
		instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();	// ������֤������

		PopulateDebugMessengerCreateInfo(debugCreateInfo);	// ��������Ϣ������������Ϣ
		instanceCreateInfo.pNext = &debugCreateInfo;	// ���õ�����Ϣ������������Ϣ
	}
	else
	{
		instanceCreateInfo.enabledLayerCount = 0;	// ������֤��
		instanceCreateInfo.pNext = nullptr;	// ��ʹ�õ�����Ϣ������
	}

	// ����Vulkanʵ��
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create Vulkan instance!");	// �������ʧ�ܣ����������Ϣ
	}
}

void VulkanRHI::InitializeDebugMessenger()
{
	if (m_enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;	// ����������Ϣ������������Ϣ�ṹ��
		PopulateDebugMessengerCreateInfo(createInfo);	// ��������Ϣ������������Ϣ
		if (VK_SUCCESS != CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger))	// ����������Ϣ������
		{
			LOG_ERROR("failed to set up debug messenger!");	// �������ʧ�ܣ����������Ϣ
		}
	}

	if (m_enableDebugUtilsLabel)	// ������õ��Թ��߱�ǩ
	{
		_vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT");	// ��ȡvkCmdBeginDebugUtilsLabelEXT����ָ��
		_vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT");	// ��ȡvkCmdEndDebugUtilsLabelEXT����ָ��
	}
}

void VulkanRHI::CreateWindowSurface()
{
	// ����Vulkan����
	if (glfwCreateWindowSurface(m_instance, m_pGLFWwindow, nullptr, &m_surface) != VK_SUCCESS)
	{
		LOG_ERROR("glfwCreateWindowSurface failed!");
	}
}

void VulkanRHI::InitializePhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;	// ��ȡ�����豸����
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);	// ��ȡ�����豸����
	if (physicalDeviceCount == 0)	// ���û�п��õ������豸
	{
		LOG_ERROR("failed to find GPUs with Vulkan support!");	// ���������Ϣ
	}
	else
	{
		//�ҵ�һ����������Ҫ����豸
		//�����ҵ���õ�
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);	// ����һ���������洢�����豸
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());	// ��ȡ�����豸�б�
		std::vector<std::pair<int, VkPhysicalDevice>> rankedPhysicalDevices;	// ����һ���������洢�����豸��������
		for (const auto& device : physicalDevices)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;	// ���������豸���Խṹ��
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);	// ��ȡ�����豸����

			int score = 0;	// ��ʼ������Ϊ0
			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // �����Կ�
			{
				score += 1000;
			}
			else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)	// �����Կ�
			{
				score += 100;
			}

			rankedPhysicalDevices.push_back({ score, device });
		}

		// �Կ���������
		std::sort(rankedPhysicalDevices.begin(),
			rankedPhysicalDevices.end(),
			[](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
				return p1 > p2;
			});

		// ���������������豸�б�ѡ���һ������Ҫ����豸
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
	m_queueIndices = FindQueueFamilies(m_physicalDevice);	// ���������豸�Ķ�����

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;	// ���д�����Ϣ
	std::set<uint32_t> queueFamilies = { m_queueIndices.m_graphicsFamily.value(), m_queueIndices.m_presentFamily.value(), m_queueIndices.m_computeFamily.value()};

	float queuePriority = 1.0f;	// �������ȼ�
	for (uint32_t queueFamily : queueFamilies)
	{
		// ���д�����Ϣ
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;	// ���ýṹ������
		queueCreateInfo.queueFamilyIndex = queueFamily;	// ���ö���������
		queueCreateInfo.queueCount = 1;	// ���ö�������
		queueCreateInfo.pQueuePriorities = &queuePriority;	// ���ö������ȼ�
		queueCreateInfos.push_back(queueCreateInfo);	// �����д�����Ϣ��ӵ�������
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};	// �����豸����
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;	// ���ø������Թ���
	physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;	// ����Ƭ����ɫ���洢��ԭ�Ӳ���
	physicalDeviceFeatures.independentBlend = VK_TRUE;	// ���ö������
	if (m_enablePointLightShadow)
	{
		physicalDeviceFeatures.geometryShader = VK_TRUE;	// ���ü�����ɫ��
	}

	VkDeviceCreateInfo deviceCreateInfo = {};	// �����߼��豸��Ϣ
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;	// ���ýṹ������
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());	// ���ö��д�����Ϣ����
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();	// ���ö��д�����Ϣ
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;	// ���������豸����
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());	// �����豸��չ����
	deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();	// �����豸��չ����
	deviceCreateInfo.enabledLayerCount = 0;	// �����豸�㣨Device Layers�������ڵ��Ի���֤���ִ� Vulkan �����ã��Ƽ�ʹ��ʵ���㣩��

	// �����߼��豸
	if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create logical device!");	// �������ʧ�ܣ����������Ϣ
	}

	VkQueue vkGraphicsQueue;	// ͼ�ζ���
	vkGetDeviceQueue(m_device, m_queueIndices.m_graphicsFamily.value(), 0, &vkGraphicsQueue);	// ��ȡͼ�ζ���
	m_graphicsQueue = new VulkanQueue();	// ����ͼ�ζ��ж���
	((VulkanQueue*)m_graphicsQueue)->SetResource(vkGraphicsQueue);	// ����ͼ�ζ�����Դ

	vkGetDeviceQueue(m_device, m_queueIndices.m_presentFamily.value(), 0, &m_presentQueue);	// ��ȡ���ֶ���

	VkQueue vkComputeQueue;	// �������
	vkGetDeviceQueue(m_device, m_queueIndices.m_computeFamily.value(), 0, &vkComputeQueue);	// ��ȡ�������
	m_computeQueue = new VulkanQueue();	// ����������ж���
	((VulkanQueue*)m_computeQueue)->SetResource(vkComputeQueue);	// ���ü��������Դ

	// ��ȡ�豸����ָ��
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

	m_depthImageFormat = (ERHIFormat)FindDepthFormat();	// ������ȸ�ʽ

	if (1)
	{

	}
}

void VulkanRHI::CreateCommandPool()
{
	// Ĭ�������
	{
		m_rhiCommandPool = new VulkanCommandPool();
		VkCommandPool vkCommandPool;
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};	// ����ش�����Ϣ
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;	// ���ýṹ������
		commandPoolCreateInfo.pNext = nullptr;	// ��ʹ����չ
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	// ��������ر�־λ�����������������
		commandPoolCreateInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily.value();	// ���ö���������
		if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &vkCommandPool) != VK_SUCCESS)
		{
			LOG_ERROR("vk create command pool");
		}
		((VulkanCommandPool*)m_rhiCommandPool)->SetResource(vkCommandPool);	// �����������Դ
	}

	// ���������
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = NULL;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;	// ��������ر�־λ��������ʱ�������
		commandPoolCreateInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily.value();
		// ��������������֧�ֶ�֡��Ⱦ
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
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};	// �������������Ϣ
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;	// ���ýṹ������
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// ���������������Ϊ������
	commandBufferAllocateInfo.commandBufferCount = 1U;	// ���������������Ϊ1

	// �����������
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
		((VulkanCommandBuffer*)m_commandBuffers[i])->SetResource(vkCommandBuffer);	// �������������Դ
	}
}

void VulkanRHI::CreateDescriptorPool()
{
	// ����DescriptorSet��Vulkan��Ӧ�ñ���Ϊ�ʲ�������DescriptorPoolӦ��
	// �㹻���������ǾͿ��Դ�DescriptorPool�з���DescriptorSet��������
	// �Ǵ�devicemmemory�з���Buffer / Imageһ����

	VkDescriptorPoolSize poolSizes[7];	// �����������ش�С����
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;	// �洢��������̬����������
	poolSizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;	// ����������
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;	// �洢����������������
	poolSizes[1].descriptorCount = 1 + 1 + 1 * m_maxVertexBlendingMeshCount;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// ͳһ����������������
	poolSizes[2].descriptorCount = 1 * m_maxMaterialCount;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// ���ͼ�����������������
	poolSizes[3].descriptorCount = 3 + 5 * m_maxMaterialCount + 1 + 1; // ImGui_ImplVulkan_CreateDeviceObjects
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;	// ���븽������������
	poolSizes[4].descriptorCount = 4 + 1 + 1 + 2;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;	// ͳһ��������̬����������
	poolSizes[5].descriptorCount = 3;
	poolSizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;	// �洢ͼ������������
	poolSizes[6].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;	// ���ýṹ������
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1 + 1 + 1 + m_maxMaterialCount + m_maxVertexBlendingMeshCount + 1 + 1; // �������������������
	poolInfo.flags = 0U;	// �������ر�־λ

	// ������������
	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS)
	{
		LOG_ERROR("failed to create descriptor pool!");	// �������ʧ�ܣ����������Ϣ
	}

	m_rhiDescriptorPool = new VulkanDescriptorPool();	// ����Vulkan�������ض���
	((VulkanDescriptorPool*)m_rhiDescriptorPool)->SetResource(m_vkDescriptorPool);	// ����Vulkan����������Դ
}

// semaphore : signal an image is ready for rendering // ready for presentation
// (m_vulkan_context._swapchain_images --> semaphores, fences)
void VulkanRHI::CreateSyncPrimitives()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};	// �ź���������Ϣ
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};	// Χ��������Ϣ
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // դ������ʼ��Ϊ�ź�

	// �����ź�����Χ��
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
}

bool VulkanRHI::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);	// ��ȡ���õ���֤������

	std::vector<VkLayerProperties> availableLayers(layerCount);	// ����һ���������洢��֤������
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());	// ��ȡ��֤������

	for (const char* layerName : m_validationLayers)
	{
		bool layerFound = false;	// ��־��������ʾ�Ƿ��ҵ���֤��
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)	// �Ƚ���֤������
			{
				layerFound = true;	// �ҵ���֤��
				break;
			}
		}

		if (!layerFound)
		{
			return false;	// ���û���ҵ�ĳ����֤�㣬�򷵻�false
		}
	}

	return RHI_SUCCESS;
}

std::vector<const char*> VulkanRHI::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;	// ��ȡGLFW�������չ����
	const char** glfwExtensions;	// ��ȡGLFW�������չ����
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);	// ��ȡGLFW�������չ����

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

// ���Իص����������ڴ���Vulkan�ĵ�����Ϣ
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
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");	// ��ȡvkCreateDebugUtilsMessengerEXT����ָ��
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
	auto queueIndices = FindQueueFamilies(physicalDevice);	// �����豸�Ķ�����
	bool isExtensionSupported = CheckDeviceExtensionSupport(physicalDevice);	// ����豸�Ƿ�֧���������չ
	bool isSwapChainAdequate = false;	// ��ʼ���������Ƿ��㹻
	if (isExtensionSupported)
	{
		ST_SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);	// ��ѯ������֧�����
		isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();	// �����������ʽ�ͳ���ģʽ����Ϊ�գ�����Ϊ�������㹻
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures;	// ����
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);	// ��ȡ�����豸����

	if (!queueIndices.isComplete() || !isSwapChainAdequate || !physicalDeviceFeatures.samplerAnisotropy)	// ��������岻�����������������������豸��֧�ָ������Թ���
	{
		return false;	// ����false����ʾ�豸���ʺ�
	}

	return true;
}

ST_QueueFamilyIndices VulkanRHI::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	ST_QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0u;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);	// ��ȡ����������
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());	// ��ȡ����������

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) // ���������֧��ͼ�β���
		{
			indices.m_graphicsFamily = i;	// ����ͼ�ζ���������
		}

		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)	// ���������֧�ּ������
		{
			indices.m_computeFamily = i;	// ���ü������������
		}

		VkBool32 presentSupport = false;	// ����֧��
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);	// ��ȡ�����豸����֧�����
		if (presentSupport)
		{
			indices.m_presentFamily = i;	// ���ó��ֶ���������
		}

		if (indices.isComplete())	// ������ж�����������������
		{
			break;	// �˳�ѭ��
		}

		++i;
	}

	return indices;
}

bool VulkanRHI::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);	// ��ȡ�豸��չ����
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);	// ���õ��豸��չ
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());	// ��ȡ�豸��չ����

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());	// ��������豸��չת��Ϊ����
	for (const auto& extension : availableExtensions)	// �������õ��豸��չ
	{
		requiredExtensions.erase(extension.extensionName);	// ��������豸��չ��ɾ�����ҵ�����չ
	}

	return requiredExtensions.empty();	// ���������豸��չ����Ϊ�գ����ʾ����������豸��չ�����ҵ�
}

ST_SwapChainSupportDetails VulkanRHI::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	ST_SwapChainSupportDetails detailsResult;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &detailsResult.capabilities);	// ��ȡ��������������

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);	// ��ȡ�����������ʽ����
	if (formatCount != 0)
	{
		detailsResult.formats.resize(formatCount);	// ���������������ʽ������С
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, detailsResult.formats.data());	// ��ȡ�����������ʽ
	}

	uint32_t presentModeCount;	// ��ȡ�������������ģʽ����
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);	// ��ȡ�������������ģʽ����
	if (presentModeCount != 0)
	{
		detailsResult.presentModes.resize(presentModeCount);	// �����������������ģʽ������С
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, detailsResult.presentModes.data());	// ��ȡ�������������ģʽ
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
	// ������ѡ��ʽ
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
	// �������õı����ʽ
	for (const auto& surfaceFormat : availableSurfaceFormats)
	{
		// TODO��ѡ��VK_FORMAT_B8G8R8A8_SRGB�����ʽ
		// ����Ҫ��Ƭ����ɫ������٤��У��
		// ѡ��VK_FORMAT_B8G8R8A8_UNORM�����ʽ
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
		// ����ѡ��VK_PRESENT_MODE_MAILBOX_KHR������ģʽ���������Լ����ӳ�
		if (VK_PRESENT_MODE_MAILBOX_KHR == presentMode)
		{
			return VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;	// Ĭ��ʹ��VK_PRESENT_MODE_FIFO_KHR��FIFOģʽ��
}

VkExtent2D VulkanRHI::ChooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// �����ǰ��չ�Ŀ�Ⱥ͸߶Ȳ���UINT32_MAX���򷵻ص�ǰ��չ
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_pGLFWwindow, &width, &height);	// ��ȡ���ڵ�֡��������С

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };	// ����ʵ����չ

		// ȷ��ʵ����չ����С�����Χ��
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

NAMESPACE_XYH_END


