#pragma once
#include <vector>
#include <Common.h>
#include <vk_mem_alloc.h>
#include <functional>
#include <memory>
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

	virtual bool IsPointLightShadowEnabled() = 0;	// �Ƿ����õ��Դ��Ӱ

	// ����ʹ���
	virtual bool AllocateCommandBuffer() = 0;	// �����������

	virtual bool AllocateDescriptorSet() = 0;	// ������������

	virtual void CreateSwapChain() = 0;	// ����������

	virtual void RecreateSwapChain() = 0;	// ���´���������

	virtual void CreateSwapChainImageViews() = 0;	// ����������ͼ����ͼ

	virtual void CreateFramebufferImageAndViews() = 0;	// ����֡����ͼ�����ͼ

	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type) = 0;	// ��ȡ�򴴽�Ĭ�ϲ�����

	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;	// ��ȡ�򴴽�mipmap������

	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode) = 0;	// ������ɫ��ģ��

	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory) = 0;	// ����������

	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;	// ��������������ʼ��
	
	virtual bool CreateBufferVMA(VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// ʹ��VMA����������

	virtual bool CreateBufferWithAlignmentVMA(
		VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIDeviceSize minAlignment,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// ʹ��VMA����������������

	virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) = 0;	// ���ƻ�����

	virtual void CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
		RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) = 0;	// ����ͼ��

	virtual void CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
		RHIImageView*& image_view) = 0;	// ����ͼ����ͼ

	virtual void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, 
		ERHIFormat texture_image_format, uint32_t miplevels = 0) = 0;	// ����ȫ��ͼ��

	virtual void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, 
		ERHIFormat texture_image_format, uint32_t miplevels) = 0;	// ������������ͼ

	virtual void CreateCommandPool() = 0;	// ���������

	virtual bool CreateCommandPool(const ST_RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) = 0;	// ���������

	virtual bool CreateDescriptorPool(const ST_RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool) = 0;	// ������������

	virtual bool CreateDescriptorSetLayout(const ST_RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout) = 0;	// ����������������

	virtual bool CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence) = 0;	// ����դ��

	virtual bool CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer) = 0;	// ����֡����

	virtual bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) = 0;	// ����ͼ�ι���

	virtual bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) = 0;	// �����������

	virtual bool CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout) = 0;	// �������߲���

	virtual bool CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass) = 0;	// ������Ⱦͨ��

	virtual bool CreateSampler(const ST_RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler) = 0;	// ����������

	virtual bool CreateSemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore) = 0;	// �����ź���


	// command and command write
	virtual bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) = 0;	// �ȴ�դ��

	virtual bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) = 0;	// ����դ��

	virtual bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) = 0;	// ���������

	virtual bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo) = 0;	// ��ʼ�������

	virtual bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer) = 0;	// �����������

	virtual void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const ST_RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents) = 0;	// ��ʼ��Ⱦͨ��

	virtual void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents) = 0;	// �л�����һ����ͨ��

	virtual void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) = 0;	// ������Ⱦͨ��

	virtual void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) = 0;	// �󶨹���

	virtual void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports) = 0;	// �����ӿ�

	virtual void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors) = 0;	// ���òü�����

	virtual void CmdBindVertexBuffersPFN(
		RHICommandBuffer* commandBuffer,
		uint32_t firstBinding,
		uint32_t bindingCount,
		RHIBuffer* const* pBuffers,
		const RHIDeviceSize* pOffsets) = 0;	// �󶨶��㻺����

	virtual void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType) = 0;	// ������������

	virtual void CmdBindDescriptorSetsPFN(
		RHICommandBuffer* commandBuffer,
		ERHIPipelineBindPoint pipelineBindPoint,
		RHIPipelineLayout* layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const RHIDescriptorSet* const* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets) = 0;	// ����������

	virtual void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;	// ��������

	virtual void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects) = 0;	// �������

	virtual bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo) = 0;	// ��ʼ�������

	virtual void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const ST_RHIBufferImageCopy* pRegions) = 0;	// ��ͼ���Ƶ�������

	virtual void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) = 0;	// ��ͼ���Ƶ�ͼ��

	virtual void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, ST_RHIBufferCopy* pRegions) = 0;	// �ӻ��������Ƶ�������

	virtual void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;	// ���ƶ���

	virtual void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;	// ���ȼ���

	virtual void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) = 0;	// ��ӵ��ȼ���

	virtual void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, 
		const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const ST_RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, 
		const ST_RHIImageMemoryBarrier* pImageMemoryBarriers) = 0;	// ��������

	virtual bool EndCommandBuffer(RHICommandBuffer* commandBuffer) = 0;	// �����������

	virtual void UpdateDescriptorSets(uint32_t descriptorWriteCount, const ST_RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const ST_RHICopyDescriptorSet* pDescriptorCopies) = 0;	// ������������

	virtual bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const ST_RHISubmitInfo* pSubmits, RHIFence* fence) = 0;	// �ύ������

	virtual bool QueueWaitIdle(RHIQueue* queue) = 0;	// ���еȴ�����

	virtual void ResetCommandPool() = 0;	// ���������

	virtual void WaitForFences() = 0;	// �ȴ�դ��


	// ��ѯ
	virtual void GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties) = 0;	// ��ȡ�����豸����

	virtual RHICommandBuffer* GetCurrentCommandBuffer() const = 0;	// ��ȡ��ǰ�������

	virtual RHICommandBuffer* const* GetCommandBufferList() const = 0; // ��ȡ��������б�

	virtual RHICommandPool* GetCommandPoor() const = 0;	// ��ȡ�����

	virtual RHIDescriptorPool* GetDescriptorPoor() const = 0;	// ��ȡ��������

	virtual RHIFence* const* GetFenceList() const = 0;	// ��ȡդ���б�

	virtual ST_QueueFamilyIndices GetQueueFamilyIndices() const = 0;	// ��ȡ����������

	virtual RHIQueue* GetGraphicsQueue() const = 0; // ��ȡͼ�ζ���

	virtual RHIQueue* GetComputeQueue() const = 0;	// ��ȡ�������

	virtual ST_RHISwapChainDesc GetSwapchainInfo() = 0;	// ��ȡ��������Ϣ

	virtual ST_RHIDepthImageDesc GetDepthImageInfo() const = 0;	// ��ȡ���ͼ����Ϣ

	virtual uint8_t GetMaxFramesInFlight() const = 0;	// ��ȡ���֡��

	virtual uint8_t GetCurrentFrameIndex() const = 0;	// ��ȡ��ǰ֡����

	virtual void SetCurrentFrameIndex(uint8_t index) = 0;	// ���õ�ǰ֡����


	// command write
	virtual RHICommandBuffer* BeginSingleTimeCommands() = 0;	// ��ʼ�����������

	virtual void EndSingleTimeCommands(RHICommandBuffer* command_buffer) = 0;	// ���������������

	virtual bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;	// ׼������Ⱦͨ��֮ǰִ�еĲ���

	virtual void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;	// �ύ��Ⱦ����

	virtual void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) = 0;	// �����¼����������

	virtual void PopEvent(RHICommandBuffer* commond_buffer) = 0;	// �����¼����������


	// ����
	virtual void Clear() = 0;	// ����RHI��Դ

	virtual void ClearSwapchain() = 0;	// ����������Դ

	virtual void DestroyDefaultSampler(ERHIDefaultSamplerType type) = 0;	// ����Ĭ�ϲ�����

	virtual void DestroyMipmappedSampler() = 0;	// ����mipmap������

	virtual void DestroyShaderModule(RHIShader* shader) = 0;	// ������ɫ��ģ��

	virtual void DestroySemaphore(RHISemaphore* semaphore) = 0;	// �����ź���

	virtual void DestroySampler(RHISampler* sampler) = 0;	// ���ٲ�����

	virtual void DestroyInstance(RHIInstance* instance) = 0;	// ����ʵ��

	virtual void DestroyImageView(RHIImageView* imageView) = 0;	// ����ͼ����ͼ

	virtual void DestroyImage(RHIImage* image) = 0;	// ����ͼ��

	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer) = 0;	// ����֡����

	virtual void DestroyFence(RHIFence* fence) = 0;	// ����դ��

	virtual void DestroyDevice() = 0;	// �����豸

	virtual void DestroyCommandPool(RHICommandPool* commandPool) = 0;	// ���������

	virtual void DestroyBuffer(RHIBuffer*& buffer) = 0;	// ���ٻ�����

	virtual void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) = 0;	// �ͷ��������


	// �ڴ�
	virtual void FreeMemory(RHIDeviceMemory*& memory) = 0;	// �ͷ��豸�ڴ�

	virtual bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) = 0;	// ӳ���豸�ڴ�

	virtual void UnmapMemory(RHIDeviceMemory* memory) = 0;	// ȡ��ӳ���豸�ڴ�

	virtual void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;	// ʹӳ����ڴ淶Χ��Ч

	virtual void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;	// ˢ��ӳ����ڴ淶Χ


	//semaphores
	virtual RHISemaphore*& GetTextureCopySemaphore(uint32_t index) = 0;	// 

};

inline RHI::~RHI() = default;

NAMESPACE_XYH_END