#include "VulkanUtil.h"
#include "../../../../Core/Macro.h"

NAMESPACE_XYH_BEGIN

VkSampler VulkanUtil::m_linearSampler = VK_NULL_HANDLE;		// 线性采样器
VkSampler VulkanUtil::m_nearestSampler = VK_NULL_HANDLE;	// 最近点采样器

VkShaderModule VulkanUtil::CreateShaderModule(VkDevice device, const std::vector<unsigned char>& shaderCode)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;	// 设置结构体类型
	shaderModuleCreateInfo.codeSize = shaderCode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}
	return shaderModule;
}

void VulkanUtil::CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;                     // use as a vertex/staging/index buffer
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families

	// 注意：这个函数只创建了缓冲区的“骨架”，即定义了其大小、用途等属性，但并没有为其分配任何内存。
	// 内存的分配和绑定是另一个独立的步骤，需要通过 vkAllocateMemory 和 vkBindBufferMemory 来完成。
	if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		LOG_ERROR("vkCreateBuffer failed!");
		return;
	}

	// 用于查询缓冲区对象的内存需求。在创建了缓冲区（vkCreateBuffer）之后，但在为其分配和绑定内存之前，必须调用此函数来了解：
	//	需要分配多少内存
	//	内存需要满足怎样的对齐要求
	//	哪些内存类型适合这个缓冲区
	VkMemoryRequirements bufferMemoryRequirements; // for allocate_info.allocationSize and allocate_info.memoryTypeIndex
	vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);

	VkMemoryAllocateInfo bufferMemoryAllocateInfo{};
	bufferMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
	bufferMemoryAllocateInfo.memoryTypeIndex = VulkanUtil::FindMemoryType(physicalDevice, bufferMemoryRequirements.memoryTypeBits, properties);

	// 分配内存，返回内存句柄
	if (vkAllocateMemory(device, &bufferMemoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		LOG_ERROR("vkAllocateMemory failed!");
		return;
	}

	// 建立了缓冲区和其实际存储内存之间的关联
	vkBindBufferMemory(device, buffer, bufferMemory, 0); // offset = 0
}

void VulkanUtil::CreateBufferAndInitialize(
	VkDevice device, 
	VkPhysicalDevice physicalDevice, 
	VkBufferUsageFlags usageFlags, 
	VkMemoryPropertyFlags memoryPropertyFlags,
	VkBuffer* pBuffer, 
	VkDeviceMemory* pMemory, 
	VkDeviceSize size, 
	void* pData, 
	int dataSize)
{
	// Create the buffer handle
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// 只创建了缓冲区的“骨架”,不实际分配内存
	if (VK_SUCCESS != vkCreateBuffer(device, &bufferCreateInfo, nullptr, pBuffer))
	{
		LOG_ERROR("create buffer buffer failed!");
		return;
	}

	// Create the memory backing up the buffer handle
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, *pBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;

	// Find a memory type index that fits the properties of the buffer
	bool memTypeFound = false;
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((memoryRequirements.memoryTypeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				memoryAllocateInfo.memoryTypeIndex = i;
				memTypeFound = true;
			}
		}
		memoryRequirements.memoryTypeBits >>= 1;
	}
	if (!memTypeFound)
	{
		LOG_ERROR("memTypeFound is nullptr");
		return;
	}

	if (VK_SUCCESS != vkAllocateMemory(device, &memoryAllocateInfo, nullptr, pMemory))
	{
		LOG_ERROR("alloc memory failed!");
		return;
	}

	if (pData != nullptr && dataSize != 0)
	{
		void* mapped;	// 映射内存的起始地址
		// 将设备内存映射到主机（CPU）可访问的地址空间
		if (VK_SUCCESS != vkMapMemory(device, *pMemory, 0, size, 0, &mapped))
		{
			LOG_ERROR("map memory failed!");
			return;
		}
		memcpy(mapped, pData, dataSize);	// 给内存赋值
		// 解除设备内存的映射关系
		vkUnmapMemory(device, *pMemory);
	}

	if (VK_SUCCESS != vkBindBufferMemory(device, *pBuffer, *pMemory, 0))
	{
		LOG_ERROR("bind memory failed!");
		return;
	}
}

VkImageView VulkanUtil::CreateImageView(
	VkDevice device,
	VkImage& image,
	VkFormat format,
	VkImageAspectFlags imageAspectFlags,
	VkImageViewType viewType,
	uint32_t layoutCount,
	uint32_t miplevels)
{
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = miplevels;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = layoutCount;

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

VkSampler VulkanUtil::GetOrCreateLinearSampler(VkPhysicalDevice physicalDevice, VkDevice device)
{
	// 如果线性采样器尚未创建，则创建一个新的
	if (m_linearSampler == VK_NULL_HANDLE)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSamplerCreateInfo samplerInfo{};

		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy; // close :1.0f
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 8.0f; // todo: m_irradiance_texture_miplevels
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &m_linearSampler) != VK_SUCCESS)
		{
			LOG_ERROR("vk create sampler");
		}
	}

	return m_linearSampler;
}

VkSampler VulkanUtil::GetOrCreateNearestSampler(VkPhysicalDevice physicalDevice, VkDevice device)
{
	// 如果最近点采样器尚未创建，则创建一个新的采样器
	if (m_nearestSampler == VK_NULL_HANDLE)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSamplerCreateInfo samplerInfo{};

		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy; // close :1.0f
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 8.0f; // todo: m_irradiance_texture_miplevels
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &m_nearestSampler) != VK_SUCCESS)
		{
			LOG_ERROR("vk create sampler");
		}
	}

	return m_nearestSampler;
}

NAMESPACE_XYH_END


