#pragma once
#include <vector>
#include <Common.h>
#include <vk_mem_alloc.h>
#include "RHIStruct.h"

NAMESPACE_XYH_BEGIN

class RHI
{
public:
	virtual ~RHI() = 0;

	virtual void Initialize() = 0;
	virtual void PrepareContext() = 0;

	virtual bool IsPointLightShadowEnabled() = 0;	// 是否启用点光源阴影

	virtual bool AllocateCommandBuffer() = 0;	// 配置命令缓冲区
	virtual bool AllocateDescriptorSet() = 0;	// 配置描述符集
	virtual void CreateSwapChain() = 0;	// 创建交换链
	virtual void RecreateSwapChain() = 0;	// 重新创建交换链
	virtual void CreateSwapChainImageViews() = 0;	// 创建交换链图像视图
	virtual void CreateFramebufferImageAndViews() = 0;	// 创建帧缓冲图像和视图
	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type) = 0;	// 获取或创建默认采样器
	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;	// 获取或创建mipmap采样器
	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode) = 0;	// 创建着色器模块
	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory) = 0;	// 创建缓冲区
	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;	// 创建缓冲区并初始化
	virtual bool createBufferVMA(VmaAllocator allocator,
		const RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// 使用VMA创建缓冲区

private:

};

RHI::RHI()
{
}

RHI::~RHI()
{
}

NAMESPACE_XYH_END