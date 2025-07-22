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
    ERHIStructureType m_type;
    const void* m_pNext;
    RHIAccessFlags m_srcAccessMask;
    RHIAccessFlags m_dstAccessMask;
};

struct ST_RHIClearDepthStencilValue
{
    float depth;
    uint32_t stencil;
};

union UN_RHIClearColorValue
{
    float float32[4];
    int32_t int32[4];
    uint32_t uint32[4];
};

union UN_RHIClearValue
{
    UN_RHIClearColorValue color;
    ST_RHIClearDepthStencilValue depthStencil;
};

struct ST_RHIClearAttachment
{
    RHIImageAspectFlags aspectMask;
    uint32_t colorAttachment;
    UN_RHIClearValue clearValue;
};

struct ST_RHIOffset2D
{
    int32_t x;
    int32_t y;
};

struct ST_RHIExtent2D
{
    uint32_t width;
    uint32_t height;
};

struct ST_RHIRect2D
{
    ST_RHIOffset2D offset;
    ST_RHIExtent2D extent;
};

struct ST_RHIClearRect 
{
    ST_RHIRect2D rect;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
};

struct ST_QueueFamilyIndices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> m_compute_family;

    bool isComplete() { return graphics_family.has_value() && present_family.has_value() && m_compute_family.has_value();; }
};

struct ST_RHISwapChainDesc
{
    ST_RHIExtent2D extent;
    ERHIFormat   image_format;
    ST_RHIViewport* viewport;
    ST_RHIRect2D* scissor;
    std::vector<RHIImageView*> imageViews;
};

struct ST_RHIViewport
{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct ST_RHIDepthImageDesc
{
    RHIImage* depth_image = VK_NULL_HANDLE;
    RHIImageView* depth_image_view = VK_NULL_HANDLE;
    ERHIFormat        depth_image_format;
};

NAMESPACE_XYH_END