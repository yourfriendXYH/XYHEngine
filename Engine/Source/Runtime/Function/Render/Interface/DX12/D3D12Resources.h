#pragma once
#include <Common.h>
//#include <Runtime/Core/Math/Vector3.h>
#include <Runtime/Function/Render/Interface/RHIResources.h>
#include "D3D12RHI.h"

NAMESPACE_XYH_BEGIN

template <typename T>
constexpr T Align(T Val, uint64_t Alignment)
{
	return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
}

enum class D3D12ResourceStateMode
{
	Default,	// 根据标志决定是否需要跟踪
	SingleState,
	MultiState,
};

class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer();
	virtual ~D3D12Buffer();

	// 将RHIBufferCreateDesc转换为D3D12_RESOURCE_DESC，并buffer的对齐值
	// UE5.8
	static void GetResourceDescAndAlignment(const RHIBufferCreateDesc& createDesc, D3D12_RESOURCE_DESC& resourceDesc, uint32_t& alignment);

private:

};

NAMESPACE_XYH_END