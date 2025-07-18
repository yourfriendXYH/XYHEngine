#pragma once
#include <Common.h>
#include "../RenderType.h"

NAMESPACE_XYH_BEGIN

#define RHI_DELETE_PTR(ptr) delete ptr; ptr = nullptr;

////////////////////��////////////////////////
class RHIBuffer {};	// ��Դ������
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
class RHIShader {};	// ��ɫ��


////////////////////�ṹ��//////////////////////////
struct RHIMemoryBarrier;	// �ڴ�����
struct RHICopyDescriptorSet;
struct RHIDescriptorImageInfo;
struct RHIDescriptorBufferInfo;
struct RHIOffset2D;
struct RHISpecializationMapEntry;
struct RHIBufferMemoryBarrier;
struct RHIImageSubresourceRange;
struct RHIImageMemoryBarrier;
struct RHIExtent2D;
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
struct RHIRect2D;
struct RHISpecializationInfo;
struct RHIStencilOpState;
struct RHIVertexInputAttributeDescription;
struct RHIVertexInputBindingDescription;
struct RHIViewport;
struct RHIRenderPassBeginInfo;
union RHIClearValue;
union RHIClearColorValue;
struct RHIClearDepthStencilValue;

////////////////////�ṹ�嶨��////////////////////////
struct RHIMemoryBarrier 
{
    ERHIStructureType m_type;
    const void* m_pNext;
    RHIAccessFlags m_srcAccessMask;
    RHIAccessFlags m_dstAccessMask;
};


NAMESPACE_XYH_END