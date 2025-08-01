#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>
#include "../RHI.h"

NAMESPACE_XYH_BEGIN

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

NAMESPACE_XYH_END