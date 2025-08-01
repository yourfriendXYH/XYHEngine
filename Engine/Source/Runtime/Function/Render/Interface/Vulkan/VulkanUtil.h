#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>

NAMESPACE_XYH_BEGIN

class VulkanUtil
{
public:
	// 创建图像视图
    static VkImageView CreateImageView(VkDevice device, VkImage& image, VkFormat format, VkImageAspectFlags image_aspect_flags, VkImageViewType view_type, uint32_t layout_count, uint32_t miplevels);

	// 创建图像
    static void CreateImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        uint32_t imageWidth,
        uint32_t imageHeight,
        VkFormat format,
        VkImageTiling imageTiling,
        VkImageUsageFlags imageUsageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkImage& image,
        VkDeviceMemory& memory,
        VkImageCreateFlags imageCreateFlags,
        uint32_t arrayLayers,
        uint32_t miplevels);

    static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlag);

};

NAMESPACE_XYH_END