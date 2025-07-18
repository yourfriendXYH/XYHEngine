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

	virtual bool IsPointLightShadowEnabled() = 0;	// �Ƿ����õ��Դ��Ӱ

	virtual bool AllocateCommandBuffer() = 0;	// �����������
	virtual bool AllocateDescriptorSet() = 0;	// ������������
	virtual void CreateSwapChain() = 0;	// ����������
	virtual void RecreateSwapChain() = 0;	// ���´���������
	virtual void CreateSwapChainImageViews() = 0;	// ����������ͼ����ͼ
	virtual void CreateFramebufferImageAndViews() = 0;	// ����֡����ͼ�����ͼ
	virtual RHISampler* GetOrCreateDefaultSampler(ERHIDefaultSamplerType type) = 0;	// ��ȡ�򴴽�Ĭ�ϲ�����
	virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;	// ��ȡ�򴴽�mipmap������
	virtual RHIShader* CreateShaderModule(const std::vector<unsigned char>& shaderCode) = 0;	// ������ɫ��ģ��
	virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory) = 0;	// ����������
	virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;	// ��������������ʼ��
	virtual bool createBufferVMA(VmaAllocator allocator,
		const RHIBufferCreateInfo* pBufferCreateInfo,
		const VmaAllocationCreateInfo* pAllocationCreateInfo,
		RHIBuffer*& pBuffer,
		VmaAllocation* pAllocation,
		VmaAllocationInfo* pAllocationInfo) = 0;	// ʹ��VMA����������

private:

};

RHI::RHI()
{
}

RHI::~RHI()
{
}

NAMESPACE_XYH_END