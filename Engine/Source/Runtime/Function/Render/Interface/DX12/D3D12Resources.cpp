#include "D3D12Resources.h"
#include <Runtime/Core/Misc/EnumClassFlags.h>

NAMESPACE_XYH_BEGIN

//Use the Euclidean method to find the GCD
[[nodiscard]] static constexpr int32_t GreatestCommonDivisor(int32_t a, int32_t b)
{
    while (b != 0)
    {
        int32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// LCM = a/gcd * b
// a and b are the number we want to find the lcm
[[nodiscard]] static constexpr int32_t LeastCommonMultiplier(int32_t a, int32_t b)
{
    int32_t CurrentGcd = GreatestCommonDivisor(a, b);
    return CurrentGcd == 0 ? 0 : (a / CurrentGcd) * b;
}

D3D12Buffer::D3D12Buffer()
{
}

D3D12Buffer::~D3D12Buffer()
{
}


void D3D12Buffer::GetResourceDescAndAlignment(const RHIBufferCreateDesc& createDesc, D3D12_RESOURCE_DESC& resourceDesc, uint32_t& alignment)
{
	// resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(createDesc.m_size);
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = createDesc.m_size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 将大小对齐为16，以便在将元素数量除以4时创建RAW缓冲区视图而不丢失任何数据
	resourceDesc.Width = Align(resourceDesc.Width, RHI_RAW_VIEW_ALIGNMENT);

	if (EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::UnorderedAccess))	// UAV
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (!EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::ShaderResource | EBufferUsageFlags::AccelerationStructure))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;	// 禁止该资源被创建为着色器资源视图
	}

	if (EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::DrawIndirect))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_NONE;
	}

	if (EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::Shared))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	}

//#if D3D12_RHI_RAYTRACING
//	if (EnumHasAnyFlags(CreateDesc.Usage, EBufferUsageFlags::AccelerationStructure))
//	{
//		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
//	}
//#endif

	if (EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::ReservedResource))
	{
		alignment = 65536;
	}
	else
	{
		// 结构化缓冲区和非字节地址缓冲区需要对齐到其步长，以确保能够通过元素偏移量正确地进行寻址。
		alignment = (createDesc.m_stride > 0) && (EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::StructuredBuffer) || !EnumHasAnyFlags(createDesc.m_usage, EBufferUsageFlags::ByteAddressBuffer | EBufferUsageFlags::DrawIndirect)) ?
			LeastCommonMultiplier(createDesc.m_stride, RHI_RAW_VIEW_ALIGNMENT) : RHI_RAW_VIEW_ALIGNMENT;
	}
}

NAMESPACE_XYH_END

