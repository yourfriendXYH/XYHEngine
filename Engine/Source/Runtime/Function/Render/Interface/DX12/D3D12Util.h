#pragma once
#include "D3D12RHI.h"

NAMESPACE_XYH_BEGIN

class D3D12Util
{
public:
	
	static void CreateResource(
		ID3D12Resource*& outResource, 
		ID3D12Device* pDevice,
		UINT width,
		UINT height,
		UINT16 mipLevels,
		DXGI_FORMAT format,
		UINT sampleCount,
		D3D12_RESOURCE_FLAGS flags
	);

private:

};

NAMESPACE_XYH_END