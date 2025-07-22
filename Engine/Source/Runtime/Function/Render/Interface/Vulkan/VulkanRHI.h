#pragma once
#include "../RHI.h"

NAMESPACE_XYH_BEGIN

class VulkanRHI final : public RHI
{
public:
	virtual void Initialize() override final;

	virtual void PrepareContext();

	virtual bool IsPointLightShadowEnabled();	// �Ƿ����õ��Դ��Ӱ

	// ����ʹ���
	virtual bool AllocateCommandBuffer();	// �����������

	virtual bool AllocateDescriptorSet();	// ������������

	virtual void CreateSwapChain();	// ����������

	virtual void RecreateSwapChain();	// ���´���������

	virtual void CreateSwapChainImageViews();	// ����������ͼ����ͼ

	virtual void CreateFramebufferImageAndViews();	// ����֡����ͼ�����ͼ

	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type);	// ��ȡ�򴴽�Ĭ�ϲ�����

	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height);	// ��ȡ�򴴽�mipmap������

	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode);	// ������ɫ��ģ��

	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory);	// ����������

	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0);	// ��������������ʼ��

	virtual bool CreateBufferVMA(VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo);	// ʹ��VMA����������

	virtual bool CreateBufferWithAlignmentVMA(
		VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIDeviceSize minAlignment,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo);	// ʹ��VMA����������������

	virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size);	// ���ƻ�����

	virtual void CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
		RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels);	// ����ͼ��

	virtual void CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
		RHIImageView*& image_view);	// ����ͼ����ͼ

	virtual void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels);	// ����ȫ��ͼ��

	virtual void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels);	// ������������ͼ

	virtual void CreateCommandPool();	// ���������

	virtual bool CreateCommandPool(const ST_RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool);	// ���������

	virtual bool CreateDescriptorPool(const ST_RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool);	// ������������

	virtual bool CreateDescriptorSetLayout(const ST_RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout);	// ����������������

	virtual bool CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence);	// ����դ��

	virtual bool CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer);	// ����֡����

	virtual bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines);	// ����ͼ�ι���

	virtual bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines);	// �����������

	virtual bool CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout);	// �������߲���

	virtual bool CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass);	// ������Ⱦͨ��

	virtual bool CreateSampler(const ST_RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler);	// ����������

	virtual bool CreateSemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore);	// �����ź���


	// command and command write
	virtual bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout);	// �ȴ�դ��

	virtual bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences);	// ����դ��

	virtual bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags);	// ���������

	virtual bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo);	// ��ʼ�������

	virtual bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer);	// �����������

	virtual void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const ST_RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents);	// ��ʼ��Ⱦͨ��

	virtual void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents);	// �л�����һ����ͨ��

	virtual void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer);	// ������Ⱦͨ��

	virtual void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline);	// �󶨹���

	virtual void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports);	// �����ӿ�

	virtual void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors);	// ���òü�����

	virtual void CmdBindVertexBuffersPFN(
		RHICommandBuffer* commandBuffer,
		uint32_t firstBinding,
		uint32_t bindingCount,
		RHIBuffer* const* pBuffers,
		const RHIDeviceSize* pOffsets);	// �󶨶��㻺����

	virtual void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType);	// ������������

	virtual void CmdBindDescriptorSetsPFN(
		RHICommandBuffer* commandBuffer,
		ERHIPipelineBindPoint pipelineBindPoint,
		RHIPipelineLayout* layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const RHIDescriptorSet* const* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets);	// ����������

	virtual void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);	// ��������

	virtual void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects);	// �������

	virtual bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo);	// ��ʼ�������

	virtual void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const ST_RHIBufferImageCopy* pRegions);	// ��ͼ���Ƶ�������

	virtual void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height);	// ��ͼ���Ƶ�ͼ��

	virtual void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, ST_RHIBufferCopy* pRegions);	// �ӻ��������Ƶ�������

	virtual void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);	// ���ƶ���

	virtual void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);	// ���ȼ���

	virtual void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset);	// ��ӵ��ȼ���

	virtual void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
		const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const ST_RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
		const ST_RHIImageMemoryBarrier* pImageMemoryBarriers);	// ��������

	virtual bool EndCommandBuffer(RHICommandBuffer* commandBuffer);	// �����������

	virtual void UpdateDescriptorSets(uint32_t descriptorWriteCount, const ST_RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const ST_RHICopyDescriptorSet* pDescriptorCopies);	// ������������

	virtual bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const ST_RHISubmitInfo* pSubmits, RHIFence* fence);	// �ύ������

	virtual bool QueueWaitIdle(RHIQueue* queue);	// ���еȴ�����

	virtual void ResetCommandPool();	// ���������

	virtual void WaitForFences();	// �ȴ�դ��


	// ��ѯ
	virtual void GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties);	// ��ȡ�����豸����

	virtual RHICommandBuffer* GetCurrentCommandBuffer() const;	// ��ȡ��ǰ�������

	virtual RHICommandBuffer* const* GetCommandBufferList() const; // ��ȡ��������б�

	virtual RHICommandPool* GetCommandPoor() const;	// ��ȡ�����

	virtual RHIDescriptorPool* GetDescriptorPoor() const;	// ��ȡ��������

	virtual RHIFence* const* GetFenceList() const;	// ��ȡդ���б�

	virtual ST_QueueFamilyIndices GetQueueFamilyIndices() const;	// ��ȡ����������

	virtual RHIQueue* GetGraphicsQueue() const; // ��ȡͼ�ζ���

	virtual RHIQueue* GetComputeQueue() const;	// ��ȡ�������

	virtual ST_RHISwapChainDesc GetSwapchainInfo();	// ��ȡ��������Ϣ

	virtual ST_RHIDepthImageDesc GetDepthImageInfo() const;	// ��ȡ���ͼ����Ϣ

	virtual uint8_t GetMaxFramesInFlight() const;	// ��ȡ���֡��

	virtual uint8_t GetCurrentFrameIndex() const;	// ��ȡ��ǰ֡����

	virtual void SetCurrentFrameIndex(uint8_t index);	// ���õ�ǰ֡����


	// command write
	virtual RHICommandBuffer* BeginSingleTimeCommands();	// ��ʼ�����������

	virtual void EndSingleTimeCommands(RHICommandBuffer* command_buffer);	// ���������������

	virtual bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain);	// ׼������Ⱦͨ��֮ǰִ�еĲ���

	virtual void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain);	// �ύ��Ⱦ����

	virtual void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color);	// �����¼����������

	virtual void PopEvent(RHICommandBuffer* commond_buffer);	// �����¼����������


	// ����
	virtual ~VulkanRHI() override final;

	virtual void Clear();	// ����RHI��Դ

	virtual void ClearSwapchain();	// ����������Դ

	virtual void DestroyDefaultSampler(ERHIDefaultSamplerType type);	// ����Ĭ�ϲ�����

	virtual void DestroyMipmappedSampler();	// ����mipmap������

	virtual void DestroyShaderModule(RHIShader* shader);	// ������ɫ��ģ��

	virtual void DestroySemaphore(RHISemaphore* semaphore);	// �����ź���

	virtual void DestroySampler(RHISampler* sampler);	// ���ٲ�����

	virtual void DestroyInstance(RHIInstance* instance);	// ����ʵ��

	virtual void DestroyImageView(RHIImageView* imageView);	// ����ͼ����ͼ

	virtual void DestroyImage(RHIImage* image);	// ����ͼ��

	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer);	// ����֡����

	virtual void DestroyFence(RHIFence* fence);	// ����դ��

	virtual void DestroyDevice();	// �����豸

	virtual void DestroyCommandPool(RHICommandPool* commandPool);	// ���������

	virtual void DestroyBuffer(RHIBuffer*& buffer);	// ���ٻ�����

	virtual void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers);	// �ͷ��������


	// �ڴ�
	virtual void FreeMemory(RHIDeviceMemory*& memory);	// �ͷ��豸�ڴ�

	virtual bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData);	// ӳ���豸�ڴ�

	virtual void UnmapMemory(RHIDeviceMemory* memory);	// ȡ��ӳ���豸�ڴ�

	virtual void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size);	// ʹӳ����ڴ淶Χ��Ч

	virtual void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size);	// ˢ��ӳ����ڴ淶Χ


	//semaphores
	virtual RHISemaphore*& GetTextureCopySemaphore(uint32_t index);	// 

private:

};

NAMESPACE_XYH_END