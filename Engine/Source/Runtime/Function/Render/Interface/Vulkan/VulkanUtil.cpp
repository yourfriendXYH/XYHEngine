#include "VulkanUtil.h"
#include <Runtime/Core/Macro.h>
#include <Runtime/Function/Render/Interface/Vulkan/VulkanRHI.h>

NAMESPACE_XYH_BEGIN

VkSampler VulkanUtil::m_linearSampler = VK_NULL_HANDLE;		// 线性采样器
VkSampler VulkanUtil::m_nearestSampler = VK_NULL_HANDLE;	// 最近点采样器
std::unordered_map<uint32_t, VkSampler> VulkanUtil::m_mipmapSamplerMap;	// mipmap采样器

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

void VulkanUtil::CopyBuffer(RHI* pRHI, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
{
	if (nullptr == pRHI)
		return;

	RHICommandBuffer* pRHICommandBuffer = static_cast<VulkanRHI*>(pRHI)->BeginSingleTimeCommands();
	VkCommandBuffer commandBuffer = static_cast<VulkanCommandBuffer*>(pRHICommandBuffer)->GetResource();

	VkBufferCopy copyRegion = { srcOffset, dstOffset, size };
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	static_cast<VulkanRHI*>(pRHI)->EndSingleTimeCommands(pRHICommandBuffer);
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

void VulkanUtil::DestroyNearestSampler(VkDevice device)
{
	vkDestroySampler(device, m_nearestSampler, nullptr);
	m_nearestSampler = VK_NULL_HANDLE;
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

void VulkanUtil::DestroyLinearSampler(VkDevice device)
{
	vkDestroySampler(device, m_linearSampler, nullptr);
	m_linearSampler = VK_NULL_HANDLE;
}

VkSampler VulkanUtil::GetOrCreateMipmapSampler(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height)
{
	if (width <= 0 || height <= 0)
		LOG_ERROR("width <= 0 || height <= 0");

	VkSampler sampler;
	uint32_t miplevels = floor(log2(std::max(width, height))) + 1;
	auto iter = m_mipmapSamplerMap.find(miplevels);
	if (iter != m_mipmapSamplerMap.end())
	{
		return iter->second;
	}
	else
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		samplerInfo.maxLod = miplevels - 1;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			LOG_ERROR("vkCreateSampler failed!");
		}
	}

	m_mipmapSamplerMap.insert(std::make_pair(miplevels, sampler));

	return VkSampler();
}

void VulkanUtil::DestroyMipmappedSampler(VkDevice device)
{
	for (auto sampler : m_mipmapSamplerMap)
	{
		vkDestroySampler(device, sampler.second, nullptr);
	}
	m_mipmapSamplerMap.clear();
}

void VulkanUtil::CreateGlobalImage(
	RHI* pRHI,
	VkImage& image,
	VkImageView& imageView,
	VmaAllocation& imageAllocation,
	uint32_t textureImageWidth,
	uint32_t textureImageHeight,
	void* pTextureImagePixels,
	ERHIFormat textureImageFormat,
	uint32_t miplevels)
{
	// 判断图片数据是否为空
	if (!pTextureImagePixels)
	{
		return;
	}

	// 确定图片格式以及图片大小
	VkDeviceSize textureByteSize = 0u;
	VkFormat vulkanImageFormat = VK_FORMAT_UNDEFINED;
	switch (textureImageFormat)
	{
	case ERHIFormat::RHI_FORMAT_R8G8B8_UNORM:
		textureByteSize = textureImageWidth * textureImageHeight * 3;
		vulkanImageFormat = VK_FORMAT_R8G8B8_UNORM;
		break;
	case ERHIFormat::RHI_FORMAT_R8G8B8_SRGB:
		textureByteSize = textureImageWidth * textureImageHeight * 3;
		vulkanImageFormat = VK_FORMAT_R8G8B8_SRGB;
		break;
	case ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM:
		textureByteSize = textureImageWidth * textureImageHeight * 4;
		vulkanImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case ERHIFormat::RHI_FORMAT_R8G8B8A8_SRGB:
		textureByteSize = textureImageWidth * textureImageHeight * 4;
		vulkanImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case ERHIFormat::RHI_FORMAT_R32_SFLOAT:
		textureByteSize = textureImageWidth * textureImageHeight * 4;
		vulkanImageFormat = VK_FORMAT_R32_SFLOAT;
		break;
	case ERHIFormat::RHI_FORMAT_R32G32_SFLOAT:
		textureByteSize = textureImageWidth * textureImageHeight * 4 * 2;
		vulkanImageFormat = VK_FORMAT_R32G32_SFLOAT;
		break;
	case ERHIFormat::RHI_FORMAT_R32G32B32_SFLOAT:
		textureByteSize = textureImageWidth * textureImageHeight * 4 * 3;
		vulkanImageFormat = VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case ERHIFormat::RHI_FORMAT_R32G32B32A32_SFLOAT:
		textureByteSize = textureImageWidth * textureImageHeight * 4 * 4;
		vulkanImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	default:
		LOG_ERROR("invalid texture_byte_size");
		break;
	}

	// 图片数据的内存创建
	VkBuffer inefficientStagingBuffer;	// 低效的临时缓冲区
	VkDeviceMemory inefficientStagingBufferMemory;	// 低效的临时缓冲区内存
	VulkanUtil::CreateBuffer(
		static_cast<VulkanRHI*>(pRHI)->m_physicalDevice,
		static_cast<VulkanRHI*>(pRHI)->m_device,
		textureByteSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		inefficientStagingBuffer,
		inefficientStagingBufferMemory);

	// 给数据赋值
	void* data;
	vkMapMemory(static_cast<VulkanRHI*>(pRHI)->m_device, inefficientStagingBufferMemory, 0, textureByteSize, 0, &data);
	memcpy(data, pTextureImagePixels, static_cast<size_t>(textureByteSize));
	vkUnmapMemory(static_cast<VulkanRHI*>(pRHI)->m_device, inefficientStagingBufferMemory);

	// generate mipmapped image
	uint32_t tempMipLevels = (miplevels != 0) ? miplevels : static_cast<uint32_t>(floor(log2(std::max(textureImageWidth, textureImageHeight)))) + 1;

	// use the vmaAllocator to allocate asset texture image
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = textureImageWidth;
	imageCreateInfo.extent.height = textureImageHeight;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = tempMipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = vulkanImageFormat;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;	// GPU only

	// 创建图像并分配内存
	vmaCreateImage(static_cast<VulkanRHI*>(pRHI)->m_assetsAllocator, &imageCreateInfo, &allocInfo, &image, &imageAllocation, NULL);

	// layout transitions -- image layout is set from none to destination
	TransitionImageLayout(pRHI, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
	// copy from staging buffer as destination
	CopyBufferToImage(pRHI, inefficientStagingBuffer, image, textureImageWidth, textureImageHeight, 1);
	// layout transitions -- image layout is set from destination to shader_read
	TransitionImageLayout(pRHI, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);

	vkDestroyBuffer(static_cast<VulkanRHI*>(pRHI)->m_device, inefficientStagingBuffer, nullptr);
	vkFreeMemory(static_cast<VulkanRHI*>(pRHI)->m_device, inefficientStagingBufferMemory, nullptr);

	// generate mipmapped image
	GenMipmappedImage(pRHI, image, textureImageWidth, textureImageHeight, tempMipLevels);

	// 创建ImageView
	imageView = CreateImageView(static_cast<VulkanRHI*>(pRHI)->m_device, image, vulkanImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, tempMipLevels);
}

void VulkanUtil::CreateCubeMap(RHI* pRHI, VkImage& image, VkImageView& imageView, VmaAllocation& imageAllocation, uint32_t textureImageWidth, uint32_t textureImageHeight, std::array<void*, 6> textureImagePixels, ERHIFormat textureImageFormat, uint32_t miplevels)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI);

	VkDeviceSize textureLayerByteSize;
	VkDeviceSize cubeByteSize;
	VkFormat vulkanImageFormat;
	switch (textureImageFormat)
	{
	case ERHIFormat::RHI_FORMAT_R8G8B8_UNORM:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 3;
		vulkanImageFormat = VK_FORMAT_R8G8B8_UNORM;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R8G8B8_SRGB:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 3;
		vulkanImageFormat = VK_FORMAT_R8G8B8_SRGB;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R8G8B8A8_UNORM:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 4;
		vulkanImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R8G8B8A8_SRGB:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 4;
		vulkanImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R32G32_SFLOAT:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 4 * 2;
		vulkanImageFormat = VK_FORMAT_R32G32_SFLOAT;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R32G32B32_SFLOAT:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 4 * 3;
		vulkanImageFormat = VK_FORMAT_R32G32B32_SFLOAT;
	}
	break;
	case ERHIFormat::RHI_FORMAT_R32G32B32A32_SFLOAT:
	{
		textureLayerByteSize = textureImageWidth * textureImageHeight * 4 * 4;
		vulkanImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	break;
	default:
	{
		textureLayerByteSize = VkDeviceSize(-1);
		LOG_ERROR("invalid texture_layer_byte_size");
		return;
	}
	break;
	}

	cubeByteSize = textureLayerByteSize * 6;

	// create cubemap texture image
	// 使用 vmaAllocator 来分配资产纹理图像
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;	// 允许一个 2D 图像被当作一个立方体贴图（Cubemap）来使用
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = static_cast<uint32_t>(textureImageWidth);
	imageCreateInfo.extent.height = static_cast<uint32_t>(textureImageHeight);
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = miplevels;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.format = vulkanImageFormat;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// 
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	vmaCreateImage(
		pVulkanRHI->m_assetsAllocator,
		&imageCreateInfo,
		&allocInfo,
		&image,
		&imageAllocation,
		nullptr
	);

	// 
	VkBuffer inefficientStagingBuffer;
	VkDeviceMemory inefficientStagingBufferMemory;
	CreateBuffer(
		pVulkanRHI->m_physicalDevice,
		pVulkanRHI->m_device,
		cubeByteSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		inefficientStagingBuffer,
		inefficientStagingBufferMemory
	);

	// 图片数据写入内存中
	void* pData = nullptr;
	vkMapMemory(pVulkanRHI->m_device, inefficientStagingBufferMemory, 0, cubeByteSize, 0, &pData);
	for (int i = 0; i < 6; i++)
	{
		memcpy((void*)(static_cast<char*>(pData) + textureLayerByteSize * i), textureImagePixels[i], static_cast<size_t>(textureLayerByteSize));
	}
	vkUnmapMemory(pVulkanRHI->m_device, inefficientStagingBufferMemory);

	// 布局过渡——图像布局从“无”状态切换至“目标”状态
	TransitionImageLayout(
		pRHI,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		6,
		miplevels,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	// 
	CopyBufferToImage(
		pRHI,
		inefficientStagingBuffer,
		image,
		static_cast<uint32_t>(textureImageWidth),
		static_cast<uint32_t>(textureImageHeight),
		6
	);

	vkDestroyBuffer(pVulkanRHI->m_device, inefficientStagingBuffer, nullptr);
	vkFreeMemory(pVulkanRHI->m_device, inefficientStagingBufferMemory, nullptr);

	// 生成mipmap
	GenerateTextureMipMaps(pRHI, image, vulkanImageFormat, textureImageWidth, textureImageHeight, 6, miplevels);

	imageView = CreateImageView(
		pVulkanRHI->m_device,
		image,
		vulkanImageFormat,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_CUBE,
		6,
		miplevels
	);
}

void VulkanUtil::GenerateTextureMipMaps(RHI* pRHI, VkImage image, VkFormat imageFormat, uint32_t textureWidth, uint32_t textureHeight, uint32_t layers, uint32_t miplevels)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI);

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(pVulkanRHI->m_physicalDevice, imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		LOG_ERROR("generateTextureMipMaps() : linear bliting not supported!");
		return;
	}

	// 命令记录
	RHICommandBuffer* pRHICommandBuffer = pVulkanRHI->BeginSingleTimeCommands();
	VkCommandBuffer commandBuffer = ((VulkanCommandBuffer*)pRHICommandBuffer)->GetResource();

	// 用于图像内存布局转换和访问同步
	VkImageMemoryBarrier barrier{};	// 图像内存屏障
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layers;
	barrier.subresourceRange.levelCount = 1; // 1 level a time

	int32_t mipWidth = textureWidth;
	int32_t mipHeight = textureHeight;

	for (uint32_t i = 1; i < miplevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		// 
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		VkImageBlit blit{};
		// src
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = layers; // miplevel i-1 to i for all layers
		// dst
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = layers;
		// 
		vkCmdBlitImage(
			commandBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR
		);

		// 
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		// 
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}
	// 
	// the last miplevel(miplevels - 1) change to shader_read
	barrier.subresourceRange.baseMipLevel = miplevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	// 
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);

	// 结束命令
	pVulkanRHI->EndSingleTimeCommands(pRHICommandBuffer);
}

void VulkanUtil::TransitionImageLayout(RHI* pRHI, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, uint32_t mipLevels, VkImageAspectFlags aspectMaskBits)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI);
	if (nullptr == pRHI)
	{
		LOG_ERROR("pRHI is nullptr");
		return;
	}

	RHICommandBuffer* pRHICommandBuffer = pVulkanRHI->BeginSingleTimeCommands();
	VkCommandBuffer commandBuffer = ((VulkanCommandBuffer*)pRHICommandBuffer)->GetResource();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectMaskBits;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	// 传输阶段
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	// for getGuidAndDepthOfMouseClickOnRenderSceneForUI() get depthimage
	else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	// for generating mipmapped image
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else
	{
		LOG_ERROR("unsupported layout transition!");
		return;
	}
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	pVulkanRHI->EndSingleTimeCommands(pRHICommandBuffer);
}

void VulkanUtil::CopyBufferToImage(RHI* pRHI, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI);
	if (nullptr == pRHI)
	{
		LOG_ERROR("pRHI is nullptr");
		return;
	}

	RHICommandBuffer* pRHICommandBuffer = pVulkanRHI->BeginSingleTimeCommands();
	VkCommandBuffer commandBuffer = ((VulkanCommandBuffer*)pRHICommandBuffer)->GetResource();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	pVulkanRHI->EndSingleTimeCommands(pRHICommandBuffer);
}

void VulkanUtil::GenMipmappedImage(RHI* pRHI, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI);
	if (nullptr == pRHI)
	{
		LOG_ERROR("pRHI is nullptr");
		return;
	}

	RHICommandBuffer* pRHICommandBuffer = pVulkanRHI->BeginSingleTimeCommands();
	VkCommandBuffer commandBuffer = ((VulkanCommandBuffer*)pRHICommandBuffer)->GetResource();

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		VkImageBlit imageBlit{};
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = std::max((int32_t)(width >> (i - 1)), 1);
		imageBlit.srcOffsets[1].y = std::max((int32_t)(height >> (i - 1)), 1);
		imageBlit.srcOffsets[1].z = 1;

		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = std::max((int32_t)(width >> i), 1);
		imageBlit.dstOffsets[1].y = std::max((int32_t)(height >> i), 1);
		imageBlit.dstOffsets[1].z = 1;

		VkImageSubresourceRange mipSubRange{};
		mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipSubRange.baseMipLevel = i;
		mipSubRange.levelCount = 1;
		mipSubRange.layerCount = 1;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = mipSubRange;

		// 
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		// 
		vkCmdBlitImage(
			commandBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageBlit,
			VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		// 
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);
	}

	VkImageSubresourceRange mipSubRange{};
	mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	mipSubRange.baseMipLevel = 0;
	mipSubRange.levelCount = mipLevels;
	mipSubRange.layerCount = 1;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = mipSubRange;

	// 
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);

	pVulkanRHI->EndSingleTimeCommands(pRHICommandBuffer);
}

NAMESPACE_XYH_END


