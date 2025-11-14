#pragma once
#include <Common.h>
#include <vulkan/vulkan.h>
#include <vector>

NAMESPACE_XYH_BEGIN

class VulkanUtil
{
public:

	// 创建着色器模块
	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<unsigned char>& shaderCode);

	static void CreateBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);

	static void CreateBufferAndInitialize(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkBuffer* pBuffer,
		VkDeviceMemory* pMemory,
		VkDeviceSize size,
		void* pData = nullptr,
		int dataSize = 0);

	// 创建图像视图
	static VkImageView CreateImageView(
		VkDevice device,
		VkImage& image,
		VkFormat format,
		VkImageAspectFlags imageAspectFlags,
		VkImageViewType viewType,
		uint32_t layoutCount,
		uint32_t miplevels);

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

	// 获取或创建线性采样器
	static VkSampler GetOrCreateLinearSampler(VkPhysicalDevice physicalDevice, VkDevice device);

	// 获取或创建最近点采样器
	static VkSampler GetOrCreateNearestSampler(VkPhysicalDevice physicalDevice, VkDevice device);

private:
	static VkSampler m_linearSampler;
	static VkSampler m_nearestSampler;
};

NAMESPACE_XYH_END