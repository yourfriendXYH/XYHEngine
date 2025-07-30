#include "VulkanUtil.h"

NAMESPACE_XYH_BEGIN

VkImageView VulkanUtil::CreateImageView(VkDevice device, VkImage& image, VkFormat format, VkImageAspectFlags image_aspect_flags, VkImageViewType view_type, uint32_t layout_count, uint32_t miplevels)
{
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = view_type;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.subresourceRange.aspectMask = image_aspect_flags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = miplevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = layout_count;

    VkImageView imageView;
    if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        return imageView;
        // todo
    }

    return imageView;
}

NAMESPACE_XYH_END


