#include "D3D12Util.h"

NAMESPACE_XYH_BEGIN

void D3D12Util::CreateResource(
	ID3D12Resource*& outResource,
	ID3D12Device* pDevice,
	D3D12_HEAP_TYPE heapType,
	UINT width, 
	UINT height, 
	UINT16 mipLevels, 
	DXGI_FORMAT format,
	UINT sampleCount,
	D3D12_TEXTURE_LAYOUT layout,
	D3D12_RESOURCE_FLAGS flags,
	D3D12_RESOURCE_STATES resStates,
	D3D12_CLEAR_VALUE* pClearValue)
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = heapType;	// 内存放的位置

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = mipLevels;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = layout;	// 拿去干嘛
	resourceDesc.Flags = flags;	// 用途
	
	pDevice->CreateCommittedResource(
		&heapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		pClearValue,
		IID_PPV_ARGS(&outResource)
	);

	return ;
}

ID3D12Resource* D3D12Util::CreateBufferObject(ID3D12GraphicsCommandList* pCommandList, ID3D12Device* pDevice, void* pData, int dataLength, D3D12_RESOURCE_STATES dstStates)
{
	ID3D12Resource* pBufferObject = nullptr;
	D3D12Util::CreateResource(
		pBufferObject,
		pDevice,
		D3D12_HEAP_TYPE_DEFAULT,
		dataLength,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		1,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr
	);

	// 写入数据
	D3D12_RESOURCE_DESC desc = pBufferObject->GetDesc();
	UINT64 totalBytes = 0;
	UINT64 rowSizeInBytes = 0;
	UINT numRows = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprint;
	pDevice->GetCopyableFootprints(
		&desc,
		0, 1, 0,
		&subresourceFootprint,
		&numRows,
		&rowSizeInBytes,
		&totalBytes
	);

	ID3D12Resource* pTempBufferObject = nullptr;
	D3D12Util::CreateResource(
		pTempBufferObject,
		pDevice,
		D3D12_HEAP_TYPE_DEFAULT,
		dataLength,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		1,
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr
	);

	BYTE* pWriteData;
	pTempBufferObject->Map(0, nullptr, reinterpret_cast<void**>(&pWriteData));
	BYTE* pDst = reinterpret_cast<BYTE*>(pWriteData + subresourceFootprint.Offset);
	BYTE* pSrcData = reinterpret_cast<BYTE*>(pData);
	for (UINT i = 0; i < numRows; ++i)
	{
		memcpy(pDst + subresourceFootprint.Footprint.RowPitch * i, pSrcData + rowSizeInBytes * i, rowSizeInBytes);
	}
	pTempBufferObject->Unmap(0, nullptr);
	pCommandList->CopyBufferRegion(pBufferObject, 0, pTempBufferObject, 0, subresourceFootprint.Footprint.Width);
	//pTempBufferObject->Release();
	D3D12_RESOURCE_BARRIER barrier = InitResourceBarrier(pBufferObject, D3D12_RESOURCE_STATE_COPY_DEST, dstStates);
	pCommandList->ResourceBarrier(1, &barrier);
	return pBufferObject;
}

D3D12_RESOURCE_BARRIER D3D12Util::InitResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES srcState, D3D12_RESOURCE_STATES dstState)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	memset(&resourceBarrier, 0, sizeof(resourceBarrier));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = pResource;
	resourceBarrier.Transition.StateBefore = srcState;
	resourceBarrier.Transition.StateAfter = dstState;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	return resourceBarrier;
}

NAMESPACE_XYH_END
