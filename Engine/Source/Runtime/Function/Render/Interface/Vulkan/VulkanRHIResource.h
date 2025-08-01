#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>
#include "../RHI.h"

NAMESPACE_XYH_BEGIN

// VulkanFence �����ڷ�װ Vulkan դ����Դ
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

// VulkanCommandPool �����ڷ�װ Vulkan �������Դ
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

// VulkanCommandBuffer �����ڷ�װ Vulkan ���������Դ
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

// VulkanDescriptorPool �����ڷ�װ Vulkan ����������Դ
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

// VulkanSemaphore �����ڷ�װ Vulkan �ź�����Դ
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

// VulkanFence �����ڷ�װ Vulkan դ����Դ
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

// VulkanImageView �����ڷ�װ Vulkan ͼ����ͼ��Դ
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

// VulkanImage �����ڷ�װ Vulkan ͼ����Դ
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