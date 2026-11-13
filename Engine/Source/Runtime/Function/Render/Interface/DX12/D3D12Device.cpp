#include "D3D12Device.h"
#include "D3D12RHI.h"

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

D3D12ContextCommon* D3D12Device::ObtainContext(ED3D12QueueType queueType)
{
    D3D12ContextCommon* context = m_queues[(uint32_t)queueType].m_objectPool.contexts.top();
    if (!context)
    {
        switch (queueType)
        {
        case XYH::ED3D12QueueType::Direct:
            context = D3D12RHI::GetD3DRHI()->CreateCommandContext(this, queueType, false);
            break;
        case XYH::ED3D12QueueType::Copy:
            break;
        case XYH::ED3D12QueueType::Async:
            break;
        default:
            break;
        }
    }

    return nullptr;
}

NAMESPACE_XYH_END
