#include "VulkanRHI.h"
#include <array>

NAMESPACE_XYH_BEGIN

void VulkanRHI::Initialize()
{
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

void VulkanRHI::CreateCommandPool()
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

bool VulkanRHI::CreateSemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore)
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

NAMESPACE_XYH_END


