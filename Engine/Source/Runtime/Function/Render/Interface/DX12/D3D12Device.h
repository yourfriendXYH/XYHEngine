#pragma once
#include <Common.h>
#include <d3d12.h>
#include "D3D12RHICommon.h"

NAMESPACE_XYH_BEGIN

class D3D12Adapter;

class D3D12Device : public D3D12AdapterChild
{
public:
	D3D12Device(/*FRHIGPUMask InGPUMask,*/ D3D12Adapter* InAdapter);
	~D3D12Device();

	ID3D12Device* GetDevice();

private:

};

NAMESPACE_XYH_END