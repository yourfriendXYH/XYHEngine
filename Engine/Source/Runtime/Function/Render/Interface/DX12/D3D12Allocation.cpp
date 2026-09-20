#include "D3D12Allocation.h"

NAMESPACE_XYH_BEGIN

ED3D12Access D3D12DefaultBufferAllocator::GetDefaultInitialD3D12Access(D3D12_HEAP_TYPE InHeapType, EBufferUsageFlags InBufferFlags, ED3D12ResourceStateMode InResourceStateMode)
{
	// Validate the create state
	if (InHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		return ED3D12Access::CopyDest;
	}
	else if (InHeapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		return ED3D12Access::GenericRead;
	}
	else if (InBufferFlags == EBufferUsageFlags::UnorderedAccess && InResourceStateMode == ED3D12ResourceStateMode::SingleState)
	{
		return ED3D12Access::UAVMask;
	}
//#if D3D12_RHI_RAYTRACING
//	else if (EnumHasAnyFlags(InBufferFlags, BUF_AccelerationStructure))
//	{
//		check(InHeapType == D3D12_HEAP_TYPE_DEFAULT);
//		return ED3D12Access::BVHRead | ED3D12Access::BVHWrite;
//	}
//#endif // D3D12_RHI_RAYTRACING
	else
	{
		return ED3D12Access::GenericRead;
	}
}

NAMESPACE_XYH_END

