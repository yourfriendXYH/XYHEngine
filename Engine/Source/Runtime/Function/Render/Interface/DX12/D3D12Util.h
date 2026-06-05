#pragma once
#include "D3D12RHI.h"

NAMESPACE_XYH_BEGIN

class D3D12Util
{
public:
	
	// ´´½¨resource
	static void CreateResource(
		ID3D12Resource*& outResource, 
		ID3D12Device* pDevice,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_DIMENSION dimension,
		UINT width,
		UINT height,
		UINT16 mipLevels,
		DXGI_FORMAT format,
		UINT sampleCount,
		D3D12_TEXTURE_LAYOUT layout,
		D3D12_RESOURCE_FLAGS flags,
		D3D12_RESOURCE_STATES resStates,
		D3D12_CLEAR_VALUE* pClearValue
	);

	static ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* pCommandList, ID3D12Device* pDevice, void* pData, int dataLength, D3D12_RESOURCE_STATES dstStates);

	static D3D12_RESOURCE_BARRIER InitResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES srcState, D3D12_RESOURCE_STATES dstState);

private:

};

NAMESPACE_XYH_END