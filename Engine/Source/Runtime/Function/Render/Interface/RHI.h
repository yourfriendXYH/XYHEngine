#pragma once
#include <vector>
#include <Common.h>
#include <vk_mem_alloc.h>
#include <functional>
#include "RHIStruct.h"

NAMESPACE_XYH_BEGIN

class WindowSystem;

struct RHIInitInfo
{
	std::shared_ptr<WindowSystem> m_pWindowSystem;
};

class RHI
{
public:
	virtual ~RHI() = 0;

	virtual void Initialize() = 0;
	virtual void PrepareContext() = 0;

	virtual bool IsPointLightShadowEnabled() = 0;	// 是否启用点光源阴影

	// 分配和创建
	virtual bool AllocateCommandBuffer() = 0;	// 配置命令缓冲区
	virtual bool AllocateDescriptorSet() = 0;	// 配置描述符集
	virtual void CreateSwapChain() = 0;	// 创建交换链
	virtual void RecreateSwapChain() = 0;	// 重新创建交换链
	virtual void CreateSwapChainImageViews() = 0;	// 创建交换链图像视图
	virtual void CreateFramebufferImageAndViews() = 0;	// 创建帧缓冲图像和视图
	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type) = 0;	// 获取或创建默认采样器
	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;	// 获取或创建mipmap采样器
	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode) = 0;	// 创建着色器模块
	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory) = 0;	// 创建缓冲区
	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;	// 创建缓冲区并初始化
	
	virtual bool CreateBufferVMA(VmaAllocator allocator,
		const RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// 使用VMA创建缓冲区

	virtual bool CreateBufferWithAlignmentVMA(
		VmaAllocator allocator,
		const RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIDeviceSize minAlignment,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// 使用VMA创建缓冲区并对齐

	virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) = 0;	// 复制缓冲区

	virtual void CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
		RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) = 0;	// 创建图像

	virtual void CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
		RHIImageView*& image_view) = 0;	// 创建图像视图

	virtual void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, 
		ERHIFormat texture_image_format, uint32_t miplevels = 0) = 0;	// 创建全局图像

	virtual void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, 
		ERHIFormat texture_image_format, uint32_t miplevels) = 0;	// 创建立方体贴图

	virtual void CreateCommandPool() = 0;	// 创建命令池

	virtual bool CreateCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) = 0;	// 创建命令池

	virtual bool CreateDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool) = 0;	// 创建描述符池

	virtual bool CreateDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout) = 0;	// 创建描述符集布局

	virtual bool CreateFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence) = 0;

	virtual bool CreateFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer) = 0;

	virtual bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) = 0;

	virtual bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) = 0;

	virtual bool CreatePipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout) = 0;

	virtual bool CreateRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass) = 0;

	virtual bool CreateSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler) = 0;

	virtual bool CreateSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore) = 0;


	// command and command write
	virtual bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) = 0;	// 等待栅栏

	virtual bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) = 0;	// 重置栅栏

	virtual bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) = 0;	// 

	virtual bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;

	virtual bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer) = 0;

	virtual void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents) = 0;

	virtual void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents) = 0;

	virtual void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) = 0;

	virtual void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) = 0;

	virtual void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports) = 0;

	virtual void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors) = 0;

	virtual void CmdBindVertexBuffersPFN(
		RHICommandBuffer* commandBuffer,
		uint32_t firstBinding,
		uint32_t bindingCount,
		RHIBuffer* const* pBuffers,
		const RHIDeviceSize* pOffsets) = 0;

	virtual void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType) = 0;

	virtual void CmdBindDescriptorSetsPFN(
		RHICommandBuffer* commandBuffer,
		ERHIPipelineBindPoint pipelineBindPoint,
		RHIPipelineLayout* layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const RHIDescriptorSet* const* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets) = 0;

	virtual void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;

	virtual void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects) = 0;

	virtual bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;
	virtual void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) = 0;

	virtual void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) = 0;

	virtual void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) = 0;

	virtual void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

	virtual void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

	virtual void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) = 0;

	virtual void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, 
		const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, 
		const RHIImageMemoryBarrier* pImageMemoryBarriers) = 0;

	virtual bool EndCommandBuffer(RHICommandBuffer* commandBuffer) = 0;

	virtual void UpdateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) = 0;

	virtual bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) = 0;

	virtual bool QueueWaitIdle(RHIQueue* queue) = 0;

	virtual void ResetCommandPool() = 0;

	virtual void WaitForFences() = 0;

	// query
	virtual void GetPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) = 0;
	virtual RHICommandBuffer* GetCurrentCommandBuffer() const = 0;
	virtual RHICommandBuffer* const* GetCommandBufferList() const = 0;
	virtual RHICommandPool* GetCommandPoor() const = 0;
	virtual RHIDescriptorPool* GetDescriptorPoor() const = 0;
	virtual RHIFence* const* GetFenceList() const = 0;
	virtual ST_QueueFamilyIndices GetQueueFamilyIndices() const = 0;
	virtual RHIQueue* GetGraphicsQueue() const = 0;
	virtual RHIQueue* GetComputeQueue() const = 0;
	virtual ST_RHISwapChainDesc GetSwapchainInfo() = 0;
	virtual ST_RHIDepthImageDesc GetDepthImageInfo() const = 0;
	virtual uint8_t GetMaxFramesInFlight() const = 0;
	virtual uint8_t GetCurrentFrameIndex() const = 0;
	virtual void SetCurrentFrameIndex(uint8_t index) = 0;

	// command write
	virtual RHICommandBuffer* BeginSingleTimeCommands() = 0;	// 
	virtual void EndSingleTimeCommands(RHICommandBuffer* command_buffer) = 0;
	virtual bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
	virtual void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
	virtual void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) = 0;
	virtual void PopEvent(RHICommandBuffer* commond_buffer) = 0;

	// destory
	virtual void Clear() = 0;	// 
	virtual void ClearSwapchain() = 0;	// 
	virtual void DestroyDefaultSampler(ERHIDefaultSamplerType type) = 0;
	virtual void DestroyMipmappedSampler() = 0;
	virtual void DestroyShaderModule(RHIShader* shader) = 0;
	virtual void DestroySemaphore(RHISemaphore* semaphore) = 0;
	virtual void DestroySampler(RHISampler* sampler) = 0;
	virtual void DestroyInstance(RHIInstance* instance) = 0;
	virtual void DestroyImageView(RHIImageView* imageView) = 0;
	virtual void DestroyImage(RHIImage* image) = 0;
	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer) = 0;
	virtual void DestroyFence(RHIFence* fence) = 0;
	virtual void DestroyDevice() = 0;
	virtual void DestroyCommandPool(RHICommandPool* commandPool) = 0;
	virtual void DestroyBuffer(RHIBuffer*& buffer) = 0;
	virtual void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) = 0;

	// memory
	virtual void FreeMemory(RHIDeviceMemory*& memory) = 0;
	virtual bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) = 0;
	virtual void UnmapMemory(RHIDeviceMemory* memory) = 0;
	virtual void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;
	virtual void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;

	//semaphores
	virtual RHISemaphore*& GetTextureCopySemaphore(uint32_t index) = 0;

};

inline RHI::~RHI() = default;

NAMESPACE_XYH_END