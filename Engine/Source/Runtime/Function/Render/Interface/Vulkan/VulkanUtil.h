#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>

NAMESPACE_XYH_BEGIN

class VulkanUtil
{
public:
	// ¥¥Ω®ÕºœÒ ”Õº
    static VkImageView CreateImageView(VkDevice device, VkImage& image, VkFormat format, VkImageAspectFlags image_aspect_flags, VkImageViewType view_type, uint32_t layout_count, uint32_t miplevels);

};

NAMESPACE_XYH_END