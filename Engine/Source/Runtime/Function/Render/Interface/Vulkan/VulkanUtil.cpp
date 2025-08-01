#include "VulkanUtil.h"
#include "../../../../Core/Macro.h"
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

void VulkanUtil::CreateImage(
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
    uint32_t miplevels)
{
    VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;    // 设置结构体类型
	imageCreateInfo.flags = imageCreateFlags;   // 图像创建标志
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;   // 图像类型为2D
    imageCreateInfo.extent.width = imageWidth;
    imageCreateInfo.extent.height = imageHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = miplevels;
    imageCreateInfo.arrayLayers = arrayLayers;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = imageTiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = imageUsageFlags;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create image!");
        return;
    }

	VkMemoryRequirements memRequirements;   // 内存需求
	vkGetImageMemoryRequirements(device, image, &memRequirements);  // 获取图像的内存需求

	// 查找合适的内存类型
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;    // 分配的内存大小
	allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, memoryPropertyFlags);    // 查找合适的内存类型索引

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        LOG_ERROR("failed to allocate image memory!");
        return;
    }

    vkBindImageMemory(device, image, memory, 0);
}

uint32_t VulkanUtil::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlag)
{
	// 获取物理设备的内存属性
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	// 遍历内存类型，查找符合要求的内存类型
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertiesFlag) == propertiesFlag)
        {
            return i;
        }
    }
    LOG_ERROR("findMemoryType error");
    return 0;
}

NAMESPACE_XYH_END


