#pragma once
#include "glew.h"
#ifdef _WIN32
#include "wglew.h"
#endif
#include <GLFW/glfw3.h>
#include <array>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"glu32.lib")

#include <Runtime/Function/Render/Interface/RHI.h>

NAMESPACE_XYH_BEGIN

class OpenGLRHI final : public RHI
{
public:

	virtual void Initialize(ST_RHIInitInfo initInfo) override final;

	virtual ~OpenGLRHI() override final;

	virtual void PrepareContext() override {};

	virtual bool IsPointLightShadowEnabled() override { return false; };	// 是否启用点光源阴影

	// 分配和创建
	virtual bool AllocateCommandBuffers(const ST_RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer*& pCommandBuffers) override { return false; };	// 配置命令缓冲区
	virtual bool AllocateDescriptorSets(const ST_RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet*& pDescriptorSets) override { return false; };	// 配置描述符集

	virtual void CreateSwapChain() override;	// 创建交换链

	virtual void RecreateSwapChain() override {};	// 重新创建交换链

	virtual void CreateSwapChainImageViews() override {};	// 创建交换链图像视图

	virtual void CreateFramebufferImageAndViews() override {};	// 创建帧缓冲图像和视图

	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type) override { return nullptr; };	// 获取或创建默认采样器

	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) override { return nullptr; };	// 获取或创建mipmap采样器

	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode) override { return nullptr; };	// 创建着色器模块

	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory) override {};	// 创建缓冲区

	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) override;	// 创建缓冲区并初始化

	virtual bool CreateBufferVMA(
		VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) override {
		return false;
	};	// 使用VMA创建缓冲区

	virtual bool CreateBufferWithAlignmentVMA(
		VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIDeviceSize minAlignment,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) override {
		return false;
	};	// 使用VMA创建缓冲区并对齐

	virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) override {};	// 复制缓冲区

	virtual void CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
		RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) override {
	};	// 创建图像

	virtual void CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
		RHIImageView*& image_view) override {
	};	// 创建图像视图

	virtual void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels = 0) override {
	};	// 创建全局图像

	virtual void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels) override {
	};	// 创建立方体贴图

	virtual void CreateCommandPool() override {};	// 创建命令池

	virtual bool CreateCommandPool(const ST_RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) override { return false; };	// 创建命令池

	virtual bool CreateDescriptorPool(const ST_RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool) override { return false; };	// 创建描述符池

	virtual bool CreateDescriptorSetLayout(const ST_RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout) override { return false; };	// 创建描述符集布局

	virtual bool CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence) override { return false; };	// 创建栅栏

	virtual bool CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer) override { return false; };	// 创建帧缓冲

	virtual bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) override { return false; };	// 创建图形管线

	virtual bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) override { return false; };	// 创建计算管线

	virtual bool CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout) override { return false; };	// 创建管线布局

	virtual bool CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass) override { return false; };	// 创建渲染通道

	virtual bool CreateSampler(const ST_RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler) override { return false; };	// 创建采样器

	virtual bool CreateRHISemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore) override { return false; };	// 创建信号量


	// command and command write
	virtual bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) override { return false; };	// 等待栅栏

	virtual bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) override { return false; };	// 重置栅栏

	virtual bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) override { return false; };	// 重置命令池

	virtual bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo) override { return false; };	// 开始命令缓冲区

	virtual bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer) override { return false; };	// 结束命令缓冲区

	virtual void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const ST_RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents) override {};	// 开始渲染通道

	virtual void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents) override {};	// 切换到下一个子通道

	virtual void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) override {};	// 结束渲染通道

	virtual void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) override {};	// 绑定管线

	virtual void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports) override {};	// 设置视口

	virtual void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors) override {};	// 设置裁剪区域

	virtual void CmdBindVertexBuffersPFN(
		RHICommandBuffer* commandBuffer,
		uint32_t firstBinding,
		uint32_t bindingCount,
		RHIBuffer* const* pBuffers,
		const RHIDeviceSize* pOffsets) override {
	};	// 绑定顶点缓冲区

	virtual void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType) override {};	// 绑定索引缓冲区

	virtual void CmdBindDescriptorSetsPFN(
		RHICommandBuffer* commandBuffer,
		ERHIPipelineBindPoint pipelineBindPoint,
		RHIPipelineLayout* layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const RHIDescriptorSet* const* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets) override {
	};	// 绑定描述符集

	virtual void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override {};	// 绘制索引

	virtual void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects) override {};	// 清除附件

	virtual bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo) override { return false; };	// 开始命令缓冲区

	virtual void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const ST_RHIBufferImageCopy* pRegions) override {};	// 从图像复制到缓冲区

	virtual void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) override {};	// 从图像复制到图像

	virtual void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, ST_RHIBufferCopy* pRegions) override {};	// 从缓冲区复制到缓冲区

	virtual void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override {};	// 绘制顶点

	virtual void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override {};	// 调度计算

	virtual void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) override {};	// 间接调度计算

	virtual void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
		const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const ST_RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
		const ST_RHIImageMemoryBarrier* pImageMemoryBarriers) override {
	};	// 管线屏障

	virtual bool EndCommandBuffer(RHICommandBuffer* commandBuffer) override { return false; };	// 结束命令缓冲区

	virtual void UpdateDescriptorSets(uint32_t descriptorWriteCount, const ST_RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const ST_RHICopyDescriptorSet* pDescriptorCopies) override {};	// 更新描述符集

	virtual bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const ST_RHISubmitInfo* pSubmits, RHIFence* fence) override { return false; };	// 提交到队列

	virtual bool QueueWaitIdle(RHIQueue* queue) override { return false; };	// 队列等待空闲

	virtual void WaitForFences() override;	// 等待栅栏

	virtual void ResetCommandPool() override;	// 重置命令池


	// 查询
	virtual void GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties) override {};	// 获取物理设备属性

	virtual RHICommandBuffer* GetCurrentCommandBuffer() const override { return nullptr; };	// 获取当前命令缓冲区

	virtual RHICommandBuffer* const* GetCommandBufferList() const override { return nullptr; }; // 获取命令缓冲区列表

	virtual RHICommandPool* GetCommandPoor() const override { return nullptr; };	// 获取命令池

	virtual RHIDescriptorPool* GetDescriptorPoor() const override { return nullptr; };	// 获取描述符池

	virtual RHIFence* const* GetFenceList() const override { return nullptr; };	// 获取栅栏列表

	virtual ST_QueueFamilyIndices GetQueueFamilyIndices() const override { return ST_QueueFamilyIndices(); };	// 获取队列族索引

	virtual RHIQueue* GetGraphicsQueue() const override { return nullptr; }; // 获取图形队列

	virtual RHIQueue* GetComputeQueue() const override { return nullptr; };	// 获取计算队列

	virtual ST_RHISwapChainDesc GetSwapchainInfo() override;	// 获取交换链信息

	virtual ST_RHIDepthImageDesc GetDepthImageInfo() const override { return ST_RHIDepthImageDesc(); };	// 获取深度图像信息

	virtual uint8_t GetMaxFramesInFlight() const override { return 0u; };	// 获取最大帧数

	virtual uint8_t GetCurrentFrameIndex() const override;	// 获取当前帧索引

	virtual void SetCurrentFrameIndex(uint8_t index) override {};	// 设置当前帧索引


	// command write
	virtual RHICommandBuffer* BeginSingleTimeCommands() override { return nullptr; };	// 开始单次命令缓冲区

	virtual void EndSingleTimeCommands(RHICommandBuffer* command_buffer) override {};	// 结束单次命令缓冲区

	virtual bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) override { return false; };	// 准备在渲染通道之前执行的操作

	virtual void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) override;	// 提交渲染操作

	virtual void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) override {};	// 开启调试标签

	virtual void PopEvent(RHICommandBuffer* commond_buffer) override {};	// 结束调试标签


	// 销毁
	virtual void Clear() override {};	// 清理RHI资源

	virtual void ClearSwapchain() override {};	// 清理交换链资源

	virtual void DestroyDefaultSampler(ERHIDefaultSamplerType type) override {};	// 销毁默认采样器

	virtual void DestroyMipmappedSampler() override {};	// 销毁mipmap采样器

	virtual void DestroyShaderModule(RHIShader* shader) override {};	// 销毁着色器模块

	virtual void DestroySemaphore(RHISemaphore* semaphore) override {};	// 销毁信号量

	virtual void DestroySampler(RHISampler* sampler) override {};	// 销毁采样器

	virtual void DestroyInstance(RHIInstance* instance) override {};	// 销毁实例

	virtual void DestroyImageView(RHIImageView* imageView) override {};	// 销毁图像视图

	virtual void DestroyImage(RHIImage* image) override {};	// 销毁图像

	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer) override {};	// 销毁帧缓冲

	virtual void DestroyFence(RHIFence* fence) override {};	// 销毁栅栏

	virtual void DestroyDevice() override {};	// 销毁设备

	virtual void DestroyCommandPool(RHICommandPool* commandPool) override {};	// 销毁命令池

	virtual void DestroyBuffer(RHIBuffer*& buffer) override {};	// 销毁缓冲区

	virtual void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) override {};	// 释放命令缓冲区


	// 内存
	virtual void FreeMemory(RHIDeviceMemory*& memory) override {};	// 释放设备内存

	virtual bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) override { return false; };	// 映射设备内存

	virtual void UnmapMemory(RHIDeviceMemory* memory) override {};	// 取消映射设备内存

	virtual void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override {};	// 使映射的内存范围无效

	virtual void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override {};	// 刷新映射的内存范围


	//semaphores
	virtual RHISemaphore*& GetTextureCopySemaphore(uint32_t index) override { return m_pRHISemaphore; };	// 

public:
	void CreateBufferObject(RHIBuffer*& pOutBuffer, GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);

private:

public:
	GLFWwindow* m_pGLFWwindow = nullptr;

private:
	// 先不管
	RHISemaphore* m_pRHISemaphore = nullptr;
};

void CheckLastOpenGLError(const char* prefix, const char* file, long line, const char* operation);
#define GLAssert(x) 	{ CheckLastOpenGLError (NULL,__FILE__, __LINE__,#x); }
#define OGL_CALL(x) do { x; GLAssert(x); } while(0)

NAMESPACE_XYH_END