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
struct RHICopyDescriptorSet;
struct RHIDescriptorImageInfo;
struct RHIDescriptorBufferInfo;
struct ST_RHIOffset2D;
struct RHISpecializationMapEntry;
struct RHIBufferMemoryBarrier;
struct RHIImageSubresourceRange;
struct RHIImageMemoryBarrier;
struct ST_RHIExtent2D;
struct RHIExtent3D;
struct RHIApplicationInfo;
struct RHIAttachmentDescription;
struct RHIBufferCopy;
struct RHIBufferCreateInfo;
struct RHIBufferImageCopy;
struct RHICommandBufferAllocateInfo;
struct RHICommandBufferBeginInfo;
struct RHICommandBufferInheritanceInfo;
struct RHICommandPoolCreateInfo;
struct RHIDescriptorPoolSize;
struct RHIDescriptorPoolCreateInfo;
struct RHIDescriptorSetAllocateInfo;
struct RHIDescriptorSetLayoutBinding;
struct RHIDescriptorSetLayoutCreateInfo;
struct RHIDeviceCreateInfo;
struct RHIDeviceQueueCreateInfo;
struct RHIExtensionProperties;
struct RHIFenceCreateInfo;
struct RHIFormatProperties;
struct RHIFramebufferCreateInfo;
struct RHIGraphicsPipelineCreateInfo;
struct RHIComputePipelineCreateInfo;
struct RHIImageBlit;
struct RHIImageCreateInfo;
struct RHIImageFormatProperties;
struct RHIImageViewCreateInfo;
struct RHIInstanceCreateInfo;
struct RHILayerProperties;
struct RHIMemoryAllocateInfo;
struct RHIMemoryHeap;
struct RHIMemoryRequirements;
struct RHIMemoryType;
struct RHIPhysicalDeviceFeatures;
struct RHIPhysicalDeviceLimits;
struct RHIPhysicalDeviceMemoryProperties;
struct RHIPhysicalDeviceProperties;
struct RHIPhysicalDeviceSparseProperties;
struct RHIPipelineColorBlendStateCreateInfo;
struct RHIPipelineDepthStencilStateCreateInfo;
struct RHIPipelineDynamicStateCreateInfo;
struct RHIPipelineInputAssemblyStateCreateInfo;
struct RHIPipelineLayoutCreateInfo;
struct RHIPipelineMultisampleStateCreateInfo;
struct RHIPipelineRasterizationStateCreateInfo;
struct RHIPipelineShaderStageCreateInfo;
struct RHIPipelineTessellationStateCreateInfo;
struct RHIPipelineVertexInputStateCreateInfo;
struct RHIPipelineViewportStateCreateInfo;
struct RHIPushConstantRange;
struct RHIQueueFamilyProperties;
struct RHIRenderPassCreateInfo;
struct RHISamplerCreateInfo;
struct RHISemaphoreCreateInfo;
struct RHIShaderModuleCreateInfo;
struct RHISubmitInfo;
struct RHISubpassDependency;
struct RHISubpassDescription;
struct RHIWriteDescriptorSet;
struct RHIOffset3D;
struct RHIAttachmentReference;
struct RHIComponentMapping;
struct RHIImageSubresourceLayers;
struct RHIPipelineColorBlendAttachmentState;
struct ST_RHIRect2D;
struct RHISpecializationInfo;
struct RHIStencilOpState;
struct RHIVertexInputAttributeDescription;
struct RHIVertexInputBindingDescription;
struct ST_RHIViewport;
struct RHIRenderPassBeginInfo;
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

struct ST_RHIClearAttachment
{
    RHIImageAspectFlags aspectMask;
    uint32_t colorAttachment;
    UN_RHIClearValue clearValue;
};

union UN_RHIClearValue
{
    UN_RHIClearColorValue color;
    ST_RHIClearDepthStencilValue depthStencil;
};

union UN_RHIClearColorValue 
{
    float float32[4];
    int32_t int32[4];
    uint32_t uint32[4];
};

struct ST_RHIClearDepthStencilValue 
{
    float depth;
    uint32_t stencil;
};

struct ST_RHIClearRect 
{
    ST_RHIRect2D rect;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
};

struct ST_RHIRect2D
{
    ST_RHIOffset2D offset;
    ST_RHIExtent2D extent;
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