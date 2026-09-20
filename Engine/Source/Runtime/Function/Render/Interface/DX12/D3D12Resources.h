#pragma once
#include <Common.h>
//#include <Runtime/Core/Math/Vector3.h>
#include <Runtime/Function/Render/Interface/RHIResources.h>
#include "D3D12Access.h"
#include <d3d12.h>

NAMESPACE_XYH_BEGIN

template <typename T>
constexpr T Align(T Val, uint64_t Alignment)
{
	return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
}

enum class ED3D12ResourceStateMode
{
	Default,	// 根据标志决定是否需要跟踪
	SingleState,
	MultiState,
};

class D3D12Resource
{
public:
	D3D12Resource() {}
	~D3D12Resource() {}

	struct FD3D12ResourceTypeHelper
	{
		FD3D12ResourceTypeHelper(const D3D12_RESOURCE_DESC& Desc, D3D12_HEAP_TYPE HeapType) :
			bSRV(!EnumHasAnyFlags(Desc.Flags, D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)),
			bDSV(EnumHasAnyFlags(Desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)),
			bRTV(EnumHasAnyFlags(Desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)),
			bUAV(EnumHasAnyFlags(Desc.Flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)),
			bWritable(bDSV || bRTV || bUAV),
			bSRVOnly(bSRV && !bWritable),
			bBuffer(Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER),
			bReadBackResource(HeapType == D3D12_HEAP_TYPE_READBACK)
		{
		}

		ED3D12Access GetOptimalInitialD3D12Access(ED3D12Access InD3D12Access, bool bAccurateWriteableStates) const
		{
			// Ignore the requested resource state for non tracked resource because RHI will assume it's always in default resource 
			// state then when a transition is required (will transition via scoped push/pop to requested state)
			if (!bSRVOnly && InD3D12Access != ED3D12Access::Unknown)
			{
				return InD3D12Access;
			}

			if (bSRVOnly)
			{
				return ED3D12Access::SRVGraphics;
			}
			else if (bBuffer && !bUAV)
			{
				return (bReadBackResource) ? ED3D12Access::CopyDest : ED3D12Access::GenericRead;
			}
			else if (bWritable && bAccurateWriteableStates)
			{
				if (bDSV)
				{
					return ED3D12Access::DSVWrite;
				}
				else if (bRTV)
				{
					return ED3D12Access::RTV;
				}
				else if (bUAV)
				{
					return ED3D12Access::UAVMask;
				}
			}

			return ED3D12Access::Common;
		}

		const uint32_t bSRV : 1;
		const uint32_t bDSV : 1;
		const uint32_t bRTV : 1;
		const uint32_t bUAV : 1;
		const uint32_t bWritable : 1;
		const uint32_t bSRVOnly : 1;
		const uint32_t bBuffer : 1;
		const uint32_t bReadBackResource : 1;
	};
};

class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer(const RHIBufferCreateDesc& InCreateDesc)
		: 
		RHIBuffer(InCreateDesc)
	{
	}
	virtual ~D3D12Buffer();

	// 将RHIBufferCreateDesc转换为D3D12_RESOURCE_DESC，并buffer的对齐值
	// UE5.8
	static void GetResourceDescAndAlignment(const RHIBufferCreateDesc& createDesc, D3D12_RESOURCE_DESC& resourceDesc, uint32_t& alignment);

public:

	ID3D12Resource* m_resource = nullptr;

	uint32_t m_bufferAlignment = 0;
};

NAMESPACE_XYH_END