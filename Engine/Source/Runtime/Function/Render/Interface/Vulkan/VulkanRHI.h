#pragma once
#include "../RHI.h"
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

NAMESPACE_XYH_BEGIN

class VulkanRHI final : public RHI
{
public:
	virtual void Initialize(ST_RHIInitInfo initInfo) override final;

	virtual void PrepareContext() override;

	virtual bool IsPointLightShadowEnabled();	// 是否启用点光源阴影

	// 分配和创建
	virtual bool AllocateCommandBuffer();	// 配置命令缓冲区

	virtual bool AllocateDescriptorSet();	// 配置描述符集

	virtual void CreateSwapChain();	// 创建交换链

	virtual void RecreateSwapChain();	// 重新创建交换链

	virtual void CreateSwapChainImageViews();	// 创建交换链图像视图

	virtual void CreateFramebufferImageAndViews();	// 创建帧缓冲图像和视图

	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type);	// 获取或创建默认采样器

	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height);	// 获取或创建mipmap采样器

	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode);	// 创建着色器模块

	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory);	// 创建缓冲区

	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0);	// 创建缓冲区并初始化

	virtual bool CreateBufferVMA(VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo);	// 使用VMA创建缓冲区

	virtual bool CreateBufferWithAlignmentVMA(
		VmaAllocator allocator,
		const ST_RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIDeviceSize minAlignment,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo);	// 使用VMA创建缓冲区并对齐

	virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size);	// 复制缓冲区

	virtual void CreateImage(uint32_t image_width, uint32_t image_height, ERHIFormat format, ERHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
		RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels);	// 创建图像

	virtual void CreateImageView(RHIImage* image, ERHIFormat format, RHIImageAspectFlags image_aspect_flags, ERHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
		RHIImageView*& image_view);	// 创建图像视图

	virtual void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels);	// 创建全局图像

	virtual void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels,
		ERHIFormat texture_image_format, uint32_t miplevels);	// 创建立方体贴图

	virtual bool CreateCommandPool(const ST_RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool);	// 创建命令池

	virtual bool CreateDescriptorPool(const ST_RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool);	// 创建描述符池

	virtual bool CreateDescriptorSetLayout(const ST_RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout);	// 创建描述符集布局

	virtual bool CreateFence(const ST_RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence);	// 创建栅栏

	virtual bool CreateFramebuffer(const ST_RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer);	// 创建帧缓冲

	virtual bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines);	// 创建图形管线

	virtual bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const ST_RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines);	// 创建计算管线

	virtual bool CreatePipelineLayout(const ST_RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout);	// 创建管线布局

	virtual bool CreateRenderPass(const ST_RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass);	// 创建渲染通道

	virtual bool CreateSampler(const ST_RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler);	// 创建采样器

	virtual bool CreateRHISemaphore(const ST_RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore);	// 创建信号量


	// command and command write
	virtual bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout);	// 等待栅栏

	virtual bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences);	// 重置栅栏

	virtual bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags);	// 重置命令池

	virtual bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo);	// 开始命令缓冲区

	virtual bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer);	// 结束命令缓冲区

	virtual void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const ST_RHIRenderPassBeginInfo* pRenderPassBegin, ERHISubpassContents contents);	// 开始渲染通道

	virtual void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, ERHISubpassContents contents);	// 切换到下一个子通道

	virtual void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer);	// 结束渲染通道

	virtual void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, ERHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline);	// 绑定管线

	virtual void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const ST_RHIViewport* pViewports);	// 设置视口

	virtual void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const ST_RHIRect2D* pScissors);	// 设置裁剪区域

	virtual void CmdBindVertexBuffersPFN(
		RHICommandBuffer* commandBuffer,
		uint32_t firstBinding,
		uint32_t bindingCount,
		RHIBuffer* const* pBuffers,
		const RHIDeviceSize* pOffsets);	// 绑定顶点缓冲区

	virtual void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, ERHIIndexType indexType);	// 绑定索引缓冲区

	virtual void CmdBindDescriptorSetsPFN(
		RHICommandBuffer* commandBuffer,
		ERHIPipelineBindPoint pipelineBindPoint,
		RHIPipelineLayout* layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const RHIDescriptorSet* const* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets);	// 绑定描述符集

	virtual void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);	// 绘制索引

	virtual void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const ST_RHIClearAttachment* pAttachments, uint32_t rectCount, const ST_RHIClearRect* pRects);	// 清除附件

	virtual bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const ST_RHICommandBufferBeginInfo* pBeginInfo);	// 开始命令缓冲区

	virtual void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const ST_RHIBufferImageCopy* pRegions);	// 从图像复制到缓冲区

	virtual void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, ERHIImageAspectFlagBits srcFlag, RHIImage* dstImage, ERHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height);	// 从图像复制到图像

	virtual void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, ST_RHIBufferCopy* pRegions);	// 从缓冲区复制到缓冲区

	virtual void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);	// 绘制顶点

	virtual void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);	// 调度计算

	virtual void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset);	// 间接调度计算

	virtual void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
		const ST_RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const ST_RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
		const ST_RHIImageMemoryBarrier* pImageMemoryBarriers);	// 管线屏障

	virtual bool EndCommandBuffer(RHICommandBuffer* commandBuffer);	// 结束命令缓冲区

	virtual void UpdateDescriptorSets(uint32_t descriptorWriteCount, const ST_RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const ST_RHICopyDescriptorSet* pDescriptorCopies);	// 更新描述符集

	virtual bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const ST_RHISubmitInfo* pSubmits, RHIFence* fence);	// 提交到队列

	virtual bool QueueWaitIdle(RHIQueue* queue);	// 队列等待空闲

	virtual void ResetCommandPool();	// 重置命令池

	virtual void WaitForFences();	// 等待栅栏


	// 查询
	virtual void GetPhysicalDeviceProperties(ST_RHIPhysicalDeviceProperties* pProperties);	// 获取物理设备属性

	virtual RHICommandBuffer* GetCurrentCommandBuffer() const;	// 获取当前命令缓冲区

	virtual RHICommandBuffer* const* GetCommandBufferList() const; // 获取命令缓冲区列表

	virtual RHICommandPool* GetCommandPoor() const;	// 获取命令池

	virtual RHIDescriptorPool* GetDescriptorPoor() const;	// 获取描述符池

	virtual RHIFence* const* GetFenceList() const;	// 获取栅栏列表

	virtual ST_QueueFamilyIndices GetQueueFamilyIndices() const;	// 获取队列族索引

	virtual RHIQueue* GetGraphicsQueue() const; // 获取图形队列

	virtual RHIQueue* GetComputeQueue() const;	// 获取计算队列

	virtual ST_RHISwapChainDesc GetSwapchainInfo();	// 获取交换链信息

	virtual ST_RHIDepthImageDesc GetDepthImageInfo() const;	// 获取深度图像信息

	virtual uint8_t GetMaxFramesInFlight() const;	// 获取最大帧数

	virtual uint8_t GetCurrentFrameIndex() const;	// 获取当前帧索引

	virtual void SetCurrentFrameIndex(uint8_t index);	// 设置当前帧索引


	// command write
	virtual RHICommandBuffer* BeginSingleTimeCommands();	// 开始单次命令缓冲区

	virtual void EndSingleTimeCommands(RHICommandBuffer* command_buffer);	// 结束单次命令缓冲区

	virtual bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain);	// 准备在渲染通道之前执行的操作

	virtual void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain);	// 提交渲染操作

	virtual void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color);	// 推送事件到命令缓冲区

	virtual void PopEvent(RHICommandBuffer* commond_buffer);	// 弹出事件从命令缓冲区


	// 销毁
	virtual ~VulkanRHI() override final;

	virtual void Clear();	// 清理RHI资源

	virtual void ClearSwapchain();	// 清理交换链资源

	virtual void DestroyDefaultSampler(ERHIDefaultSamplerType type);	// 销毁默认采样器

	virtual void DestroyMipmappedSampler();	// 销毁mipmap采样器

	virtual void DestroyShaderModule(RHIShader* shader);	// 销毁着色器模块

	virtual void DestroySemaphore(RHISemaphore* semaphore);	// 销毁信号量

	virtual void DestroySampler(RHISampler* sampler);	// 销毁采样器

	virtual void DestroyInstance(RHIInstance* instance);	// 销毁实例

	virtual void DestroyImageView(RHIImageView* imageView);	// 销毁图像视图

	virtual void DestroyImage(RHIImage* image);	// 销毁图像

	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer);	// 销毁帧缓冲

	virtual void DestroyFence(RHIFence* fence);	// 销毁栅栏

	virtual void DestroyDevice();	// 销毁设备

	virtual void DestroyCommandPool(RHICommandPool* commandPool);	// 销毁命令池

	virtual void DestroyBuffer(RHIBuffer*& buffer);	// 销毁缓冲区

	virtual void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers);	// 释放命令缓冲区


	// 内存
	virtual void FreeMemory(RHIDeviceMemory*& memory);	// 释放设备内存

	virtual bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData);	// 映射设备内存

	virtual void UnmapMemory(RHIDeviceMemory* memory);	// 取消映射设备内存

	virtual void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size);	// 使映射的内存范围无效

	virtual void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size);	// 刷新映射的内存范围


	//semaphores
	virtual RHISemaphore*& GetTextureCopySemaphore(uint32_t index);	// 

private:
	void CreateInstance();	// 创建Vulkan实例
	void InitializeDebugMessenger();	// 初始化调试消息处理器

	// 为 Vulkan 提供一个与平台（Windows / macOS / Linux）兼容的渲染目标，使得 Vulkan 能够将图形渲染到 GLFW 创建的窗口中。
	void CreateWindowSurface();	// 创建窗口表面

	void InitializePhysicalDevice();	// 初始化物理设备

	void CreateLogicalDevice();	// 创建逻辑设备

	void CreateCommandPool() override;;	// 创建命令池

	void CreateCommandBuffers();	// 创建命令缓冲区

	void CreateDescriptorPool();	// 创建描述符池

	void CreateSyncPrimitives();	// 创建同步原语

	void CreateAssetAllocator();	// 创建资源分配器


	// Vulkan 的验证层（Validation Layer）是一种调试工具，可以帮助开发者在开发阶段捕捉 API 的误用、潜在的 bug 和性能问题。常用的验证层如 "VK_LAYER_KHRONOS_validation"。
	bool CheckValidationLayerSupport();	// 检查验证层支持

	std::vector<const char*> GetRequiredExtensions();	// 获取所需的扩展列表

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);	// 填充调试消息处理器创建信息

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);	// 创建调试工具消息处理器

	bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);	// 检查物理设备是否适合

	ST_QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);	// 查找队列族

	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);	// 检查设备扩展支持

	ST_SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);	// 查询交换链支持

	VkFormat FindDepthFormat();	// 查找深度格式
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);	// 查找支持的格式

	VkSurfaceFormatKHR ChooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);	// 选择交换链表面格式
	VkPresentModeKHR ChooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes);	// 选择交换链呈现模式
	VkExtent2D ChooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);	// 选择交换链范围

public:

	ST_RHIViewport m_viewport;	// 视口属性
	ST_RHIRect2D m_scissor;	// 裁剪区域
	GLFWwindow* m_pGLFWwindow;	// GLFW窗口

	VkInstance m_instance = nullptr;	// Vulkan实例
	VkSurfaceKHR m_surface = nullptr;	// Vulkan表面
	VkPhysicalDevice m_physicalDevice;	// Vulkan物理设备

	ST_QueueFamilyIndices m_queueIndices;	// 队列族索引

	VkDevice m_device = nullptr;	// Vulkan逻辑设备
	RHIQueue* m_graphicsQueue = nullptr;	// Vulkan图形队列
	VkQueue m_presentQueue = nullptr;	// Vulkan呈现队列
	RHIQueue* m_computeQueue = nullptr;	// Vulkan计算队列

	ERHIFormat m_depthImageFormat = RHI_FORMAT_UNDEFINED;	// 深度图像格式

	// 默认命令池
	RHICommandPool* m_rhiCommandPool;
	// 其他命令池
	static uint8_t const s_maxFramesInFlight = 3u;	// 最大帧数
	uint8_t m_currentFrameIndex = 0u;	// 当前帧索引
	VkCommandPool m_commandPools[s_maxFramesInFlight];	// 其他命令池
	// 3个临时命令缓冲区
	VkCommandBuffer m_vkCommandBuffers[s_maxFramesInFlight];	// 命令缓冲区列表
	RHICommandBuffer* m_commandBuffers[s_maxFramesInFlight];	// RHI命令缓冲区列表

	VkDescriptorPool m_vkDescriptorPool;	// 全局描述符池
	RHIDescriptorPool* m_rhiDescriptorPool;	// RHI描述符池

	// VkSemaphore 和 VkFence 是两种同步原语（Synchronization Primitives），用于协调 GPU 和 CPU 之间的执行顺序，防止资源竞争和数据竞争
	RHISemaphore* m_imageAvailableForTexturescopySemaphores[s_maxFramesInFlight];	// 图像可用信号量
	VkSemaphore m_imageAvailableForRenderSemaphores[s_maxFramesInFlight];	// 渲染图像可用信号量
	VkSemaphore m_imageFinishedForPresentationSemaphores[s_maxFramesInFlight];	// 图像完成呈现信号量
	VkFence m_isFrameInFlightFences[s_maxFramesInFlight];	// 帧在执行中的栅栏
	RHIFence* m_rhiIsFrameInFlightFences[s_maxFramesInFlight];	// RHI帧在执行中的栅栏

	VkSwapchainKHR m_swapchain = nullptr;	// Vulkan交换链
	std::vector<VkImage> m_swapchainImages;	// Vulkan交换链图像
	ERHIFormat m_swapchainImageFormat = RHI_FORMAT_UNDEFINED;	// Vulkan交换链图像格式
	ST_RHIExtent2D m_swapchainExtent;	// Vulkan交换链宽高
	std::vector<RHIImageView*> m_swapchainImageViews;	// Vulkan交换链图像视图
	uint32_t m_currentSwapchainImageIndex;

	RHIImage* m_depthImage;	// 深度图像
	VkDeviceMemory m_depthImageMemory = nullptr;	// 深度图像内存
	RHIImageView* m_depthImageView;	// 深度图像视图

	// 资源分配器使用 VMA library
	VmaAllocator m_assetsAllocator;

	// Vulkan函数指针
	PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;	// 开始调试工具标签
	PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;	// 结束调试工具标签
	PFN_vkWaitForFences         _vkWaitForFences;	// 等待栅栏
	PFN_vkResetFences           _vkResetFences;		// 重置栅栏
	PFN_vkResetCommandPool      _vkResetCommandPool;
	PFN_vkBeginCommandBuffer    _vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer      _vkEndCommandBuffer;
	PFN_vkCmdBeginRenderPass    _vkCmdBeginRenderPass;
	PFN_vkCmdNextSubpass        _vkCmdNextSubpass;
	PFN_vkCmdEndRenderPass      _vkCmdEndRenderPass;
	PFN_vkCmdBindPipeline       _vkCmdBindPipeline;
	PFN_vkCmdSetViewport        _vkCmdSetViewport;
	PFN_vkCmdSetScissor         _vkCmdSetScissor;
	PFN_vkCmdBindVertexBuffers  _vkCmdBindVertexBuffers;
	PFN_vkCmdBindIndexBuffer    _vkCmdBindIndexBuffer;
	PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
	PFN_vkCmdDrawIndexed        _vkCmdDrawIndexed;
	PFN_vkCmdClearAttachments   _vkCmdClearAttachments;

private:

	bool m_enableValidationLayers = true;	// 是否启用验证层
	bool m_enableDebugUtilsLabel = true;	// 是否启用调试工具标签
	bool m_enablePointLightShadow = true;	// 是否启用点光源阴影

	uint32_t m_vulkanApiVersion = VK_API_VERSION_1_0;	// Vulkan API版本
	const std::vector<char const*> m_validationLayers{ "VK_LAYER_KHRONOS_validation" };	// 验证层列表

	VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;	// 调试消息处理器

	std::vector<char const*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };	// 设备扩展列表

	uint32_t m_maxVertexBlendingMeshCount = 256u;	// 最大顶点混合网格数量
	uint32_t m_maxMaterialCount = 256u;	// 最大材质数量
};

NAMESPACE_XYH_END