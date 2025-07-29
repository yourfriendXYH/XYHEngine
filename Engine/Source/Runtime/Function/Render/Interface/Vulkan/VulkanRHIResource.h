#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>
#include "../RHI.h"

NAMESPACE_XYH_BEGIN

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

NAMESPACE_XYH_END