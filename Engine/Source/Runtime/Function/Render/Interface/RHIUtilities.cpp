#include "RHIUtilities.h"
#include <Runtime/Core/Misc/EnumClassFlags.h>

NAMESPACE_XYH_BEGIN

ERHIAccess RHIGetDefaultResourceState(EBufferUsageFlags InUsage, bool bInHasInitialData)
{
	// Default reading state is different per buffer type
	ERHIAccess DefaultReadingState = ERHIAccess::Unknown;
	if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::IndexBuffer))
	{
		DefaultReadingState = ERHIAccess::VertexOrIndexBuffer;
	}
	if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::VertexBuffer))
	{
		// Could be vertex buffer or normal DataBuffer
		DefaultReadingState = DefaultReadingState | ERHIAccess::VertexOrIndexBuffer | ERHIAccess::SRVMask;
	}
	if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::StructuredBuffer))
	{
		DefaultReadingState = DefaultReadingState | ERHIAccess::SRVMask;
	}
	if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::AccelerationStructure))
	{
		DefaultReadingState = DefaultReadingState | ERHIAccess::BVHRead;
	}

	// Vertex and index buffers might not have the BUF_ShaderResource flag set and just assume
	// they are readable by default
	ERHIAccess ResourceState = (!EnumHasAnyFlags(DefaultReadingState, ERHIAccess::VertexOrIndexBuffer)) ? ERHIAccess::Unknown : DefaultReadingState;

	// SRV when we have initial data because we can sample the buffer then
	if (bInHasInitialData)
	{
		ResourceState = DefaultReadingState;
	}
	else
	{
		if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::UnorderedAccess))
		{
			ResourceState = ERHIAccess::UAVMask;
		}
		else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::AccelerationStructure))
		{
			ResourceState = ERHIAccess::BVHWrite;
		}
		else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::ShaderResource))
		{
			ResourceState = DefaultReadingState | ERHIAccess::SRVMask;
		}
	}

	return ResourceState;
}

NAMESPACE_XYH_END


