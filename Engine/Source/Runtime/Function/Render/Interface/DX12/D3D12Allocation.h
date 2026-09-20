#pragma once
#include <Common.h>
#include "D3D12Access.h"
#include <Runtime/Function/Render/Interface/RHIDefinitions.h>
#include "D3D12Resources.h"

NAMESPACE_XYH_BEGIN

class D3D12DefaultBufferAllocator/* : public FD3D12DeviceChild, public FD3D12MultiNodeGPUObject*/
{
public:
	//FD3D12DefaultBufferAllocator(FD3D12Device* InParent, FRHIGPUMask VisibleNodes);

	//// Grab a buffer from the available buffers or create a new buffer if none are available
	//void AllocDefaultResource(
	//	D3D12_HEAP_TYPE InHeapType,
	//	const D3D12_RESOURCE_DESC& pDesc,
	//	EBufferUsageFlags InBufferUsage,
	//	ED3D12ResourceStateMode InResourceStateMode,
	//	ED3D12Access InCreateD3D12Access,
	//	FD3D12ResourceLocation& ResourceLocation,
	//	uint32 Alignment,
	//	const TCHAR* Name);

	//void FreeDefaultBufferPools();
	//void BeginFrame(FD3D12ContextArray const& Contexts);
	//void CleanupFreeBlocks(uint64 InFrameLag);
	//void UpdateMemoryStats();

	//static bool IsPlacedResource(D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment);
	static ED3D12Access GetDefaultInitialD3D12Access(D3D12_HEAP_TYPE InHeapType, EBufferUsageFlags InBufferFlags, ED3D12ResourceStateMode InResourceStateMode);

private:

	//FD3D12BufferPool* CreateBufferPool(D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment);

	//TArray<FD3D12BufferPool*> DefaultBufferPools;
	//FCriticalSection CS;
	//HeapId TraceHeapId;
};

NAMESPACE_XYH_END