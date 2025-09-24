#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>
#include "../RHI.h"

NAMESPACE_XYH_BEGIN

// VulkanBuffer 类用于封装 Vulkan 缓冲区资源
class VulkanBuffer : public RHIBuffer
{
public:
	void SetResource(VkBuffer res)
	{
		m_resource = res;
	}
	VkBuffer GetResource() const
	{
		return m_resource;
	}
private:
	VkBuffer m_resource;
};

// VulkanFence 类用于封装 Vulkan 栅栏资源
class VulkanQueue : public RHIQueue
{
public:
	void SetResource(VkQueue res)
	{
		m_resource = res;
	}
	VkQueue GetResource() const
	{
		return m_resource;
	}

private:
	VkQueue m_resource;
};

// VulkanCommandPool 类用于封装 Vulkan 命令池资源
class VulkanCommandPool : public RHICommandPool
{
public:
	void SetResource(VkCommandPool res)
	{
		m_resource = res;
	}
	VkCommandPool GetResource() const
	{
		return m_resource;
	}

private:
	VkCommandPool m_resource;
};

// VulkanCommandBuffer 类用于封装 Vulkan 命令缓冲区资源
class VulkanCommandBuffer : public RHICommandBuffer
{
public:
	void SetResource(VkCommandBuffer res)
	{
		m_resource = res;
	}
	const VkCommandBuffer GetResource() const
	{
		return m_resource;
	}

private:
	VkCommandBuffer m_resource;
};

// VulkanDescriptorPool 类用于封装 Vulkan 描述符池资源
class VulkanDescriptorPool : public RHIDescriptorPool
{
public:
	void SetResource(VkDescriptorPool res)
	{
		m_resource = res;
	}
	VkDescriptorPool GetResource() const
	{
		return m_resource;
	}
private:
	VkDescriptorPool m_resource;
};

class VulkanDeviceMemory : public RHIDeviceMemory
{
public:
	void SetResource(VkDeviceMemory res)
	{
		m_resource = res;
	}
	VkDeviceMemory GetResource() const
	{
		return m_resource;
	}
private:
	VkDeviceMemory m_resource;
};

// VulkanSemaphore 类用于封装 Vulkan 信号量资源
class VulkanSemaphore : public RHISemaphore
{
public:
	void SetResource(VkSemaphore res)
	{
		m_resource = res;
	}
	VkSemaphore& GetResource()
	{
		return m_resource;
	}
private:
	VkSemaphore m_resource;
};

// VulkanFence 类用于封装 Vulkan 栅栏资源
class VulkanFence : public RHIFence
{
public:
	void SetResource(VkFence res)
	{
		m_resource = res;
	}
	VkFence GetResource() const
	{
		return m_resource;
	}
private:
	VkFence m_resource;
};

// VulkanImageView 类用于封装 Vulkan 图像视图资源
class VulkanImageView : public RHIImageView
{
public:
	void SetResource(VkImageView res)
	{
		m_resource = res;
	}
	VkImageView GetResource() const
	{
		return m_resource;
	}
private:
	VkImageView m_resource;
};

// VulkanImage 类用于封装 Vulkan 图像资源
class VulkanImage : public RHIImage
{
public:
	void SetResource(VkImage res)
	{
		m_resource = res;
	}
	VkImage& GetResource()
	{
		return m_resource;
	}
private:
	VkImage m_resource;
};

// 渲染通道
class VulkanRenderPass : public RHIRenderPass
{
public:
	void SetResource(VkRenderPass res)
	{
		m_resource = res;
	}
	VkRenderPass GetResource() const
	{
		return m_resource;
	}
private:
	VkRenderPass m_resource;
};

// 采样器 封装
class VulkanSampler : public RHISampler
{
public:
	void SetResource(VkSampler res)
	{
		m_resource = res;
	}
	VkSampler GetResource() const
	{
		return m_resource;
	}
private:
	VkSampler m_resource;
};

// 描述符集布局 封装
class VulkanDescriptorSetLayout : public RHIDescriptorSetLayout
{
public:
	void SetResource(VkDescriptorSetLayout res)
	{
		m_resource = res;
	}
	VkDescriptorSetLayout GetResource() const
	{
		return m_resource;
	}
private:
	VkDescriptorSetLayout m_resource;
};

// 着色器 封装
class VulkanShader : public RHIShader
{
public:
	void SetResource(VkShaderModule res)
	{
		m_resource = res;
	}
	VkShaderModule GetResource() const
	{
		return m_resource;
	}
private:
	VkShaderModule m_resource;
};

// 管线布局 封装
class VulkanPipelineLayout : public RHIPipelineLayout
{
public:
	void SetResource(VkPipelineLayout res)
	{
		m_resource = res;
	}
	VkPipelineLayout GetResource() const
	{
		return m_resource;
	}
private:
	VkPipelineLayout m_resource;
};

// 管线 封装
class VulkanPipeline : public RHIPipeline
{
public:
	void SetResource(VkPipeline res)
	{
		m_resource = res;
	}
	VkPipeline GetResource() const
	{
		return m_resource;
	}
private:
	VkPipeline m_resource;
};

// 管线缓存 封装
class VulkanPipelineCache : public RHIPipelineCache
{
public:
	void SetResource(VkPipelineCache res)
	{
		m_resource = res;
	}
	VkPipelineCache GetResource() const
	{
		return m_resource;
	}
private:
	VkPipelineCache m_resource;
};

// 描述符集 封装
class VulkanDescriptorSet : public RHIDescriptorSet
{
public:
	void SetResource(VkDescriptorSet res)
	{
		m_resource = res;
	}
	VkDescriptorSet GetResource() const
	{
		return m_resource;
	}
private:
	VkDescriptorSet m_resource;
};

// 帧缓冲区 封装
class VulkanFramebuffer : public RHIFramebuffer
{
public:
	void SetResource(VkFramebuffer res)
	{
		m_resource = res;
	}
	VkFramebuffer GetResource() const
	{
		return m_resource;
	}
private:
	VkFramebuffer m_resource;
};

NAMESPACE_XYH_END