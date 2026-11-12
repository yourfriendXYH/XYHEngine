#include "D3D12Device.h"

NAMESPACE_XYH_BEGIN

D3D12Device::D3D12Device(D3D12Adapter* InAdapter)
    :
    D3D12AdapterChild(InAdapter)
{
}

D3D12Device::~D3D12Device()
{
}

ID3D12Device* D3D12Device::GetDevice()
{
    return GetParentAdapter()->GetD3DDevice();
}

NAMESPACE_XYH_END
