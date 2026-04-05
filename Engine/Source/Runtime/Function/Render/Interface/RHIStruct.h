#pragma once
#include <Common.h>
#include <optional>
#include "../RenderType.h"

NAMESPACE_XYH_BEGIN

#define RHI_DELETE_PTR(ptr) delete ptr; ptr = nullptr;

////////////////////类////////////////////////
class RHIBuffer {};	// 资源缓冲区
class RHIBufferView {};
class RHICommandBuffer {};
class RHICommandPool {};
class RHIDescriptorPool {};
class RHIDescriptorSet {};
class RHIDescriptorSetLayout {};
class RHIDevice {};
class RHIDeviceMemory {};
class RHIEvent {};
class RHIFence {};
class RHIFramebuffer {};
class RHIImage {};
class RHIImageView {};
class RHIInstance {};
class RHIQueue {};
class RHIPhysicalDevice {};
class RHIPipeline {};
class RHIPipelineCache {};
class RHIPipelineLayout {};
class RHIRenderPass {};
class RHISampler {};
class RHISemaphore {};
class RHIShader {};	// 着色器


////////////////////结构体//////////////////////////
struct ST_RHIMemoryBarrier;	// 内存屏障
struct ST_RHICopyDescriptorSet;
struct ST_RHIDescriptorImageInfo;
struct ST_RHIDescriptorBufferInfo;
struct ST_RHIOffset2D;
struct ST_RHISpecializationMapEntry;
struct ST_RHIBufferMemoryBarrier;
struct ST_RHIImageSubresourceRange;
struct ST_RHIImageMemoryBarrier;
struct ST_RHIExtent2D;
struct ST_RHIExtent3D;
struct ST_RHIApplicationInfo;
struct ST_RHIAttachmentDescription;
struct ST_RHIBufferCopy;
struct ST_RHIBufferCreateInfo;
struct ST_RHIBufferImageCopy;
struct ST_RHICommandBufferAllocateInfo;
struct ST_RHICommandBufferBeginInfo;
struct ST_RHICommandBufferInheritanceInfo;
struct ST_RHICommandPoolCreateInfo;
struct ST_RHIDescriptorPoolSize;
struct ST_RHIDescriptorPoolCreateInfo;
struct ST_RHIDescriptorSetAllocateInfo;
struct ST_RHIDescriptorSetLayoutBinding;
struct ST_RHIDescriptorSetLayoutCreateInfo;
struct ST_RHIDeviceCreateInfo;
struct ST_RHIDeviceQueueCreateInfo;
struct ST_RHIExtensionProperties;
struct ST_RHIFenceCreateInfo;
struct ST_RHIFormatProperties;
struct ST_RHIFramebufferCreateInfo;
struct ST_RHIGraphicsPipelineCreateInfo;
struct ST_RHIComputePipelineCreateInfo;
struct ST_RHIImageBlit;
struct ST_RHIImageCreateInfo;
struct ST_RHIImageFormatProperties;
struct ST_RHIImageViewCreateInfo;
struct ST_RHIInstanceCreateInfo;
struct ST_RHILayerProperties;
struct ST_RHIMemoryAllocateInfo;
struct ST_RHIMemoryHeap;
struct ST_RHIMemoryRequirements;
struct ST_RHIMemoryType;
struct ST_RHIPhysicalDeviceFeatures;
struct ST_RHIPhysicalDeviceLimits;
struct ST_RHIPhysicalDeviceMemoryProperties;
struct ST_RHIPhysicalDeviceProperties;
struct ST_RHIPhysicalDeviceSparseProperties;
struct ST_RHIPipelineColorBlendStateCreateInfo;
struct ST_RHIPipelineDepthStencilStateCreateInfo;
struct ST_RHIPipelineDynamicStateCreateInfo;
struct ST_RHIPipelineInputAssemblyStateCreateInfo;
struct ST_RHIPipelineLayoutCreateInfo;
struct ST_RHIPipelineMultisampleStateCreateInfo;
struct ST_RHIPipelineRasterizationStateCreateInfo;
struct ST_RHIPipelineShaderStageCreateInfo;
struct ST_RHIPipelineTessellationStateCreateInfo;
struct ST_RHIPipelineVertexInputStateCreateInfo;
struct ST_RHIPipelineViewportStateCreateInfo;
struct ST_RHIPushConstantRange;
struct ST_RHIQueueFamilyProperties;
struct ST_RHIRenderPassCreateInfo;
struct ST_RHISamplerCreateInfo;
struct ST_RHISemaphoreCreateInfo;
struct ST_RHIShaderModuleCreateInfo;
struct ST_RHISubmitInfo;
struct ST_RHISubpassDependency;
struct ST_RHISubpassDescription;
struct ST_RHIWriteDescriptorSet;
struct ST_RHIOffset3D;
struct ST_RHIAttachmentReference;
struct ST_RHIComponentMapping;
struct ST_RHIImageSubresourceLayers;
struct ST_RHIPipelineColorBlendAttachmentState;
struct ST_RHIRect2D;
struct ST_RHISpecializationInfo;
struct ST_RHIStencilOpState;
struct ST_RHIVertexInputAttributeDescription;
struct ST_RHIVertexInputBindingDescription;
struct ST_RHIViewport;
struct ST_RHIRenderPassBeginInfo;
union UN_RHIClearValue;
union UN_RHIClearColorValue;
struct ST_RHIClearDepthStencilValue;

////////////////////结构体定义////////////////////////
struct ST_RHIMemoryBarrier
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIAccessFlags m_srcAccessMask;
	RHIAccessFlags m_dstAccessMask;
};

struct ST_RHICopyDescriptorSet
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIDescriptorSet* m_pSrcSet;
	uint32_t m_srcBinding;
	uint32_t m_srcArrayElement;
	RHIDescriptorSet* m_pDstSet;
	uint32_t m_dstBinding;
	uint32_t m_dstArrayElement;
	uint32_t m_descriptorCount;
};

struct ST_RHIOffset2D
{
	int32_t m_x;
	int32_t m_y;
};

// 特化映射条目
struct ST_RHISpecializationMapEntry
{
	uint32_t m_constantID;
	uint32_t m_offset;
	size_t m_size;
};

struct ST_RHIExtent2D
{
	uint32_t m_width;
	uint32_t m_height;
};

// 附件描述
struct ST_RHIAttachmentDescription
{
	RHIAttachmentDescriptionFlags m_flags;	// 附件描述标志
	ERHIFormat m_format;	// 附件格式
	ERHISampleCountFlagBits m_samples;	// 采样数
	ERHIAttachmentLoadOp m_loadOp;	// 加载操作
	ERHIAttachmentStoreOp m_storeOp;	// 存储操作
	ERHIAttachmentLoadOp m_stencilLoadOp;	// 模板加载操作
	ERHIAttachmentStoreOp m_stencilStoreOp;	// 模板存储操作
	ERHIImageLayout m_initialLayout;	// 初始布局
	ERHIImageLayout m_finalLayout;	// 最终布局
};

// 管线颜色混合附件状态
struct ST_RHIPipelineColorBlendAttachmentState
{
	RHIBool32 m_blendEnable;
	ERHIBlendFactor m_srcColorBlendFactor;
	ERHIBlendFactor m_dstColorBlendFactor;
	ERHIBlendOp m_colorBlendOp;
	ERHIBlendFactor m_srcAlphaBlendFactor;
	ERHIBlendFactor m_dstAlphaBlendFactor;
	ERHIBlendOp m_alphaBlendOp;
	RHIColorComponentFlags m_colorWriteMask;
};

// 管线颜色混合状态创建信息
struct ST_RHIPipelineColorBlendStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineColorBlendStateCreateFlags m_flags;
	RHIBool32 m_logicOpEnable;
	ERHILogicOp m_logicOp;
	uint32_t m_attachmentCount;
	const ST_RHIPipelineColorBlendAttachmentState* m_pAttachments;
	float m_blendConstants[4];
};

struct ST_RHIStencilOpState
{
	ERHIStencilOp m_failOp;
	ERHIStencilOp m_passOp;
	ERHIStencilOp m_depthFailOp;
	ERHICompareOp m_compareOp;
	uint32_t m_compareMask;
	uint32_t m_writeMask;
	uint32_t m_reference;
};

// 管线深度模板状态创建信息
struct ST_RHIPipelineDepthStencilStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineDepthStencilStateCreateFlags m_flags;
	RHIBool32 m_depthTestEnable;
	RHIBool32 m_depthWriteEnable;
	ERHICompareOp m_depthCompareOp;
	RHIBool32 m_depthBoundsTestEnable;
	RHIBool32 m_stencilTestEnable;
	ST_RHIStencilOpState m_front;
	ST_RHIStencilOpState m_back;
	float m_minDepthBounds;
	float m_maxDepthBounds;
};

struct ST_RHIRect2D
{
	ST_RHIOffset2D m_offset;    // 矩形区域的左上角坐标
	ST_RHIExtent2D m_extent;    // 矩形区域的大小
};

// 顶点输入属性描述
struct ST_RHIVertexInputAttributeDescription
{
	uint32_t m_location;
	uint32_t m_binding;
	ERHIFormat m_format;
	uint32_t m_offset;
};

struct ST_RHIClearRect
{
	ST_RHIRect2D m_rect;
	uint32_t m_baseArrayLayer;
	uint32_t m_layerCount;
};

struct ST_QueueFamilyIndices    // 队列族索引
{
	std::optional<uint32_t> m_graphicsFamily;    // 图形队列族索引
	std::optional<uint32_t> m_presentFamily;     // 呈现队列族索引
	std::optional<uint32_t> m_computeFamily;   // 计算队列族索引

	bool isComplete() { return m_graphicsFamily.has_value() && m_presentFamily.has_value() && m_computeFamily.has_value();; }
};

struct ST_SwapChainSupportDetails   // 交换链支持细节
{
	VkSurfaceCapabilitiesKHR m_capabilities;   // 交换链的能力
	std::vector<VkSurfaceFormatKHR> m_formats;    // 交换链支持的表面格式
	std::vector<VkPresentModeKHR> m_presentModes;   // 交换链支持的呈现模式
};

struct ST_RHISwapChainDesc
{
	ST_RHIExtent2D m_extent;    // 交换链的宽度和高度
	ERHIFormat m_imageFormat;   // 交换链图像的格式
	ST_RHIViewport* m_pViewport;    // 视口
	ST_RHIRect2D* m_pScissor;   // 裁剪区域
	std::vector<RHIImageView*> m_imageViews;  // 交换链图像视图
};

struct ST_RHIRenderPassCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIRenderPassCreateFlags m_flags;
	uint32_t m_attachmentCount;
	const ST_RHIAttachmentDescription* m_pAttachments;
	uint32_t m_subpassCount;
	const ST_RHISubpassDescription* m_pSubpasses;
	uint32_t m_dependencyCount;
	const ST_RHISubpassDependency* m_pDependencies;
};

struct ST_RHISubmitInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	uint32_t m_waitSemaphoreCount;
	RHISemaphore** m_pWaitSemaphores;
	const RHIPipelineStageFlags* m_pWaitDstStageMask;
	uint32_t m_commandBufferCount;
	RHICommandBuffer* const* m_pCommandBuffers;
	uint32_t m_signalSemaphoreCount;
	const RHISemaphore** m_pSignalSemaphores;
};

struct ST_RHISubpassDependency
{
	uint32_t m_srcSubpass;
	uint32_t m_dstSubpass;
	RHIPipelineStageFlags m_srcStageMask;
	RHIPipelineStageFlags m_dstStageMask;
	RHIAccessFlags m_srcAccessMask;
	RHIAccessFlags m_dstAccessMask;
	RHIDependencyFlags m_dependencyFlags;
};

// 子通道描述
struct ST_RHISubpassDescription
{
	RHISubpassDescriptionFlags m_flags;
	ERHIPipelineBindPoint m_pipelineBindPoint;	// 管线绑定点
	uint32_t m_inputAttachmentCount = 0u;	// 输入附件数量
	const ST_RHIAttachmentReference* m_pInputAttachments;
	uint32_t m_colorAttachmentCount = 0u;	// 输出附件数量
	const ST_RHIAttachmentReference* m_pColorAttachments;
	const ST_RHIAttachmentReference* m_pResolveAttachments = nullptr;
	const ST_RHIAttachmentReference* m_pDepthStencilAttachment = nullptr;
	uint32_t m_preserveAttachmentCount = 0u;	// 保留附件数量
	const uint32_t* m_pPreserveAttachments;	// 保留附件索引
};

// 附件引用
struct ST_RHIAttachmentReference
{
	uint32_t m_attachment;
	ERHIImageLayout m_layout;	// 附件布局
};

// 顶点输入属性描述
struct ST_RHIVertexInputBindingDescription
{
	uint32_t m_binding;
	uint32_t m_stride;
	ERHIVertexInputRate m_inputRate;
};

struct ST_RHIViewport
{
	float m_x;
	float m_y;
	float m_width;
	float m_height;
	float m_minDepth;
	float m_maxDepth;
};

struct ST_RHIRenderPassBeginInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext = nullptr;
	RHIRenderPass* m_pRenderPass;
	RHIFramebuffer* m_pFramebuffer;
	ST_RHIRect2D m_renderArea;
	uint32_t m_clearValueCount;
	const UN_RHIClearValue* m_pClearValues;
};

struct ST_RHIClearDepthStencilValue
{
	float m_depth;
	uint32_t m_stencil;
};

union UN_RHIClearColorValue
{
	float m_float32[4];
	int32_t m_int32[4];
	uint32_t m_uint32[4];
};

union UN_RHIClearValue
{
	UN_RHIClearColorValue m_color;
	ST_RHIClearDepthStencilValue m_depthStencil;
};

struct ST_RHIClearAttachment
{
	RHIImageAspectFlags m_aspectMask;
	uint32_t m_colorAttachment;
	UN_RHIClearValue m_clearValue;
};

struct ST_RHIDepthImageDesc
{
	RHIImage* m_depthImage = VK_NULL_HANDLE;
	RHIImageView* m_depthImageView = VK_NULL_HANDLE;
	ERHIFormat m_depthImageFormat;
};

// 描述符集布局绑定
struct ST_RHIDescriptorSetLayoutBinding
{
	uint32_t m_binding;
	ERHIDescriptorType m_descriptorType;
	uint32_t m_descriptorCount;
	RHIShaderStageFlags m_stageFlags;
	RHISampler* const* m_pImmutableSamplers = nullptr;	// 不可变采样器
};

// 描述符集布局创建信息
struct ST_RHIDescriptorSetLayoutCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIDescriptorSetLayoutCreateFlags m_flags;
	uint32_t m_bindingCount;
	const ST_RHIDescriptorSetLayoutBinding* m_pBindings;
};

// 管线布局创建信息
struct ST_RHIPipelineLayoutCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineLayoutCreateFlags m_flags;
	uint32_t m_setLayoutCount;
	RHIDescriptorSetLayout* const* m_pSetLayouts;
	uint32_t m_pushConstantRangeCount;
	const ST_RHIPushConstantRange* m_pPushConstantRanges;
};

// 推送常量范围
struct ST_RHIPushConstantRange
{
	RHIShaderStageFlags m_stageFlags;
	uint32_t m_offset;
	uint32_t m_size;
};

// 管线着色器阶段创建信息
struct ST_RHIPipelineShaderStageCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineShaderStageCreateFlags m_flags;
	ERHIShaderStageFlagBits m_stage;
	RHIShader* m_module;
	const char* m_pName;
	const ST_RHISpecializationInfo* m_pSpecializationInfo;	// 特殊化信息
};

// 特殊化映射条目
struct ST_RHISpecializationInfo
{
	uint32_t m_mapEntryCount;
	const ST_RHISpecializationMapEntry** m_pMapEntries;
	size_t m_dataSize;
	const void* m_pData;
};

// 管线顶点输入状态创建信息
struct ST_RHIPipelineVertexInputStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineVertexInputStateCreateFlags m_flags;
	uint32_t m_vertexBindingDescriptionCount;
	const ST_RHIVertexInputBindingDescription* m_pVertexBindingDescriptions;
	uint32_t m_vertexAttributeDescriptionCount;
	const ST_RHIVertexInputAttributeDescription* m_pVertexAttributeDescriptions;
};

// 管线输入装配状态创建信息
struct ST_RHIPipelineInputAssemblyStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineInputAssemblyStateCreateFlags m_flags;
	ERHIPrimitiveTopology m_topology;	// 图元拓扑类型
	RHIBool32 m_primitiveRestartEnable;
};

// 管线视口状态创建信息
struct ST_RHIPipelineViewportStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineViewportStateCreateFlags m_flags;
	uint32_t m_viewportCount;
	const ST_RHIViewport* m_pViewports;
	uint32_t m_scissorCount;
	const ST_RHIRect2D* m_pScissors;
};

// 管线光栅化状态创建信息
struct ST_RHIPipelineRasterizationStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineRasterizationStateCreateFlags m_flags;	// 管线光栅化状态创建标志
	RHIBool32 m_depthClampEnable;	// 是否启用深度裁剪
	RHIBool32 m_rasterizerDiscardEnable;	// 是否丢弃光栅化阶段
	ERHIPolygonMode m_polygonMode;	// 多边形模式
	RHICullModeFlags m_cullMode;	// 剔除模式
	ERHIFrontFace m_frontFace;
	RHIBool32 m_depthBiasEnable;	// 是否启用深度偏移
	float m_depthBiasConstantFactor;	// 常量深度偏移因子
	float m_depthBiasClamp;	// 深度偏移夹具
	float m_depthBiasSlopeFactor;	// 斜率深度偏移因子
	float m_lineWidth;	// 线宽
};

// 管线多重采样状态创建信息
struct ST_RHIPipelineMultisampleStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineMultisampleStateCreateFlags m_flags;
	ERHISampleCountFlagBits m_rasterizationSamples;
	RHIBool32 m_sampleShadingEnable;
	float m_minSampleShading;
	const RHISampleMask** m_pSampleMask;
	RHIBool32 m_alphaToCoverageEnable;	// 是否启用Alpha到覆盖
	RHIBool32 m_alphaToOneEnable;
};

// 管线动态状态创建信息
struct ST_RHIPipelineDynamicStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineDynamicStateCreateFlags m_flags;
	uint32_t m_dynamicStateCount;
	const ERHIDynamicState* m_pDynamicStates;
};

// 管线曲面细分状态创建信息
struct ST_RHIPipelineTessellationStateCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineTessellationStateCreateFlags m_flags;
	uint32_t m_patchControlPoints;	// 曲面控制点数
};

// 图形管线创建信息
struct ST_RHIGraphicsPipelineCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineCreateFlags m_flags;
	uint32_t m_stageCount;
	const ST_RHIPipelineShaderStageCreateInfo* m_pStages;
	const ST_RHIPipelineVertexInputStateCreateInfo* m_pVertexInputState;
	const ST_RHIPipelineInputAssemblyStateCreateInfo* m_pInputAssemblyState;	// 输入装配状态
	const ST_RHIPipelineTessellationStateCreateInfo* m_pTessellationState;	// 曲面细分状态
	const ST_RHIPipelineViewportStateCreateInfo* m_pViewportState;
	const ST_RHIPipelineRasterizationStateCreateInfo* m_pRasterizationState;
	const ST_RHIPipelineMultisampleStateCreateInfo* m_pMultisampleState;
	const ST_RHIPipelineDepthStencilStateCreateInfo* m_pDepthStencilState;
	const ST_RHIPipelineColorBlendStateCreateInfo* m_pColorBlendState;
	const ST_RHIPipelineDynamicStateCreateInfo* m_pDynamicState;
	RHIPipelineLayout* m_pLayout;
	RHIRenderPass* m_pRenderPass;
	uint32_t m_subpass;
	RHIPipeline* m_pBasePipelineHandle;
	int32_t m_basePipelineIndex;
};

// 计算管线创建信息
struct ST_RHIComputePipelineCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIPipelineCreateFlags m_flags;
	ST_RHIPipelineShaderStageCreateInfo* m_pStages;	// 
	RHIPipelineLayout* m_pLayout;
	RHIPipeline* m_pBasePipelineHandle;
	int32_t m_basePipelineIndex;
};

// 描述符图像信息
struct ST_RHIDescriptorImageInfo
{
	RHISampler* m_pSampler;
	RHIImageView* m_pImageView;
	ERHIImageLayout m_imageLayout;
};

// 描述符缓冲区信息
struct ST_RHIDescriptorBufferInfo
{
	RHIBuffer* m_pBuffer;
	RHIDeviceSize m_offset;
	RHIDeviceSize m_range;
};

struct ST_RHIWriteDescriptorSet
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIDescriptorSet* m_pDstSet;	// 目标描述符集
	uint32_t m_dstBinding;
	uint32_t m_dstArrayElement;
	uint32_t m_descriptorCount;
	ERHIDescriptorType m_descriptorType;
	ST_RHIDescriptorImageInfo* m_pImageInfo = nullptr;
	ST_RHIDescriptorBufferInfo* m_pBufferInfo = nullptr;
	RHIBufferView* m_pTexelBufferView = nullptr;
};

struct ST_RHIFenceCreateInfo	// 
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIFenceCreateFlags m_flags;
};

struct ST_RHIFramebufferCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIFramebufferCreateFlags m_flags;
	RHIRenderPass* m_pRenderPass;
	uint32_t m_attachmentCount;
	RHIImageView* const* m_pAttachments;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_layers;
};

struct ST_RHICommandBufferAllocateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHICommandPool* m_pCommandPool;	// 命令池
	ERHICommandBufferLevel m_level;	// 命令缓冲区级别
	uint32_t m_commandBufferCount;	// 命令缓冲区数量
};

// 命令缓冲开始信息
struct ST_RHICommandBufferBeginInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHICommandBufferUsageFlags m_flags;
	const ST_RHICommandBufferInheritanceInfo* m_pInheritanceInfo;	// 继承信息
};

struct ST_RHICommandBufferInheritanceInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIRenderPass* m_pRenderPass;
	uint32_t m_subpass;
	RHIFramebuffer* m_pFramebuffer;
	RHIBool32 m_occlusionQueryEnable;
	RHIQueryControlFlags m_queryFlags;
	RHIQueryPipelineStatisticFlags m_pipelineStatistics;
};

struct ST_RHIDescriptorSetAllocateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIDescriptorPool* m_pDescriptorPool;	// 描述符池
	uint32_t m_descriptorSetCount;	// 描述符集数量
	const RHIDescriptorSetLayout* const* m_pSetLayouts;	// 描述符集布局
};

struct ST_RHISamplerCreateInfo
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHISamplerCreateFlags m_flags;
	ERHIFilter m_magFilter;
	ERHIFilter m_minFilter;
	ERHISamplerMipmapMode m_mipmapMode;
	ERHISamplerAddressMode m_addressModeU;
	ERHISamplerAddressMode m_addressModeV;
	ERHISamplerAddressMode m_addressModeW;
	float m_mipLodBias;
	RHIBool32 m_anisotropyEnable;
	float m_maxAnisotropy;
	RHIBool32 m_compareEnable;
	ERHICompareOp m_compareOp;
	float m_minLod;
	float m_maxLod;
	ERHIBorderColor m_borderColor;
	RHIBool32 m_unnormalizedCoordinates;
};

// 图像子资源范围
struct ST_RHIImageSubresourceRange
{
	RHIImageAspectFlags m_aspectMask;
	uint32_t m_baseMipLevel;
	uint32_t m_levelCount;
	uint32_t m_baseArrayLayer;
	uint32_t m_layerCount;
};

// 图像内存屏障
struct ST_RHIImageMemoryBarrier
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIAccessFlags m_srcAccessMask;
	RHIAccessFlags m_dstAccessMask;
	ERHIImageLayout m_oldLayout;
	ERHIImageLayout m_newLayout;
	uint32_t m_srcQueueFamilyIndex;
	uint32_t m_dstQueueFamilyIndex;
	RHIImage* m_pImage;
	ST_RHIImageSubresourceRange m_subresourceRange;
};

struct ST_RHIBufferMemoryBarrier
{
	ERHIStructureType m_sType;
	const void* m_pNext;
	RHIAccessFlags m_srcAccessMask;
	RHIAccessFlags m_dstAccessMask;
	uint32_t m_srcQueueFamilyIndex;
	uint32_t m_dstQueueFamilyIndex;
	RHIBuffer* m_pBuffer;
	RHIDeviceSize m_offset;
	RHIDeviceSize m_size;
};

struct ST_RHIBufferCopy
{
	RHIDeviceSize m_srcOffset;
	RHIDeviceSize m_dstOffset;
	RHIDeviceSize m_size;
};

struct ST_RHIPhysicalDeviceLimits
{
	uint32_t m_maxImageDimension1D;
	uint32_t m_maxImageDimension2D;
	uint32_t m_maxImageDimension3D;
	uint32_t m_maxImageDimensionCube;
	uint32_t m_maxImageArrayLayers;
	uint32_t m_maxTexelBufferElements;
	uint32_t m_maxUniformBufferRange;
	uint32_t m_maxStorageBufferRange;
	uint32_t m_maxPushConstantsSize;
	uint32_t m_maxMemoryAllocationCount;
	uint32_t m_maxSamplerAllocationCount;
	RHIDeviceSize m_bufferImageGranularity;
	RHIDeviceSize m_sparseAddressSpaceSize;
	uint32_t m_maxBoundDescriptorSets;
	uint32_t m_maxPerStageDescriptorSamplers;
	uint32_t m_maxPerStageDescriptorUniformBuffers;
	uint32_t m_maxPerStageDescriptorStorageBuffers;
	uint32_t m_maxPerStageDescriptorSampledImages;
	uint32_t m_maxPerStageDescriptorStorageImages;
	uint32_t m_maxPerStageDescriptorInputAttachments;
	uint32_t m_maxPerStageResources;
	uint32_t m_maxDescriptorSetSamplers;
	uint32_t m_maxDescriptorSetUniformBuffers;
	uint32_t m_maxDescriptorSetUniformBuffersDynamic;
	uint32_t m_maxDescriptorSetStorageBuffers;
	uint32_t m_maxDescriptorSetStorageBuffersDynamic;
	uint32_t m_maxDescriptorSetSampledImages;
	uint32_t m_maxDescriptorSetStorageImages;
	uint32_t m_maxDescriptorSetInputAttachments;
	uint32_t m_maxVertexInputAttributes;
	uint32_t m_maxVertexInputBindings;
	uint32_t m_maxVertexInputAttributeOffset;
	uint32_t m_maxVertexInputBindingStride;
	uint32_t m_maxVertexOutputComponents;
	uint32_t m_maxTessellationGenerationLevel;
	uint32_t m_maxTessellationPatchSize;
	uint32_t m_maxTessellationControlPerVertexInputComponents;
	uint32_t m_maxTessellationControlPerVertexOutputComponents;
	uint32_t m_maxTessellationControlPerPatchOutputComponents;
	uint32_t m_maxTessellationControlTotalOutputComponents;
	uint32_t m_maxTessellationEvaluationInputComponents;
	uint32_t m_maxTessellationEvaluationOutputComponents;
	uint32_t m_maxGeometryShaderInvocations;
	uint32_t m_maxGeometryInputComponents;
	uint32_t m_maxGeometryOutputComponents;
	uint32_t m_maxGeometryOutputVertices;
	uint32_t m_maxGeometryTotalOutputComponents;
	uint32_t m_maxFragmentInputComponents;
	uint32_t m_maxFragmentOutputAttachments;
	uint32_t m_maxFragmentDualSrcAttachments;
	uint32_t m_maxFragmentCombinedOutputResources;
	uint32_t m_maxComputeSharedMemorySize;
	uint32_t m_maxComputeWorkGroupCount[3];
	uint32_t m_maxComputeWorkGroupInvocations;
	uint32_t m_maxComputeWorkGroupSize[3];
	uint32_t m_subPixelPrecisionBits;
	uint32_t m_subTexelPrecisionBits;
	uint32_t m_mipmapPrecisionBits;
	uint32_t m_maxDrawIndexedIndexValue;
	uint32_t m_maxDrawIndirectCount;
	float m_maxSamplerLodBias;
	float m_maxSamplerAnisotropy;
	uint32_t m_maxViewports;
	uint32_t m_maxViewportDimensions[2];
	float m_viewportBoundsRange[2];
	uint32_t m_viewportSubPixelBits;
	size_t m_minMemoryMapAlignment;
	RHIDeviceSize m_minTexelBufferOffsetAlignment;
	RHIDeviceSize m_minUniformBufferOffsetAlignment;
	RHIDeviceSize m_minStorageBufferOffsetAlignment;
	int32_t m_minTexelOffset;
	uint32_t m_maxTexelOffset;
	int32_t m_minTexelGatherOffset;
	uint32_t m_maxTexelGatherOffset;
	float m_minInterpolationOffset;
	float m_maxInterpolationOffset;
	uint32_t m_subPixelInterpolationOffsetBits;
	uint32_t m_maxFramebufferWidth;
	uint32_t m_maxFramebufferHeight;
	uint32_t m_maxFramebufferLayers;
	RHISampleCountFlags m_framebufferColorSampleCounts;
	RHISampleCountFlags m_framebufferDepthSampleCounts;
	RHISampleCountFlags m_framebufferStencilSampleCounts;
	RHISampleCountFlags m_framebufferNoAttachmentsSampleCounts;
	uint32_t m_maxColorAttachments;
	RHISampleCountFlags m_sampledImageColorSampleCounts;
	RHISampleCountFlags m_sampledImageIntegerSampleCounts;
	RHISampleCountFlags m_sampledImageDepthSampleCounts;
	RHISampleCountFlags m_sampledImageStencilSampleCounts;
	RHISampleCountFlags m_storageImageSampleCounts;
	uint32_t m_maxSampleMaskWords;
	RHIBool32 m_timestampComputeAndGraphics;
	float m_timestampPeriod;
	uint32_t m_maxClipDistances;
	uint32_t m_maxCullDistances;
	uint32_t m_maxCombinedClipAndCullDistances;
	uint32_t m_discreteQueuePriorities;
	float m_pointSizeRange[2];
	float m_lineWidthRange[2];
	float m_pointSizeGranularity;
	float m_lineWidthGranularity;
	RHIBool32 m_strictLines;
	RHIBool32 m_standardSampleLocations;
	RHIDeviceSize m_optimalBufferCopyOffsetAlignment;
	RHIDeviceSize m_optimalBufferCopyRowPitchAlignment;
	RHIDeviceSize m_nonCoherentAtomSize;
};

struct ST_RHIPhysicalDeviceSparseProperties
{
	RHIBool32 m_residencyStandard2DBlockShape;
	RHIBool32 m_residencyStandard2DMultisampleBlockShape;
	RHIBool32 m_residencyStandard3DBlockShape;
	RHIBool32 m_residencyAlignedMipSize;
	RHIBool32 m_residencyNonResidentStrict;
};

struct ST_RHIPhysicalDeviceProperties
{
	uint32_t m_apiVersion;
	uint32_t m_driverVersion;
	uint32_t m_vendorID;
	uint32_t m_deviceID;
	ERHIPhysicalDeviceType m_deviceType;
	char m_deviceName[RHI_MAX_PHYSICAL_DEVICE_NAME_SIZE];
	uint8_t m_pipelineCacheUUID[RHI_UUID_SIZE];
	ST_RHIPhysicalDeviceLimits m_limits;
	ST_RHIPhysicalDeviceSparseProperties m_sparseProperties;
};

struct ST_RHIBufferCreateInfo
{
	ERHIStructureType sType;
	const void* pNext;
	RHIBufferCreateFlags flags;
	RHIDeviceSize size;
	RHIBufferUsageFlags usage;
	ERHISharingMode sharingMode;
	uint32_t queueFamilyIndexCount;
	const uint32_t* pQueueFamilyIndices;
};

NAMESPACE_XYH_END