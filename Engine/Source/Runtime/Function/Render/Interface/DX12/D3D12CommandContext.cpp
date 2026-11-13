#include "D3D12CommandContext.h"
#include "D3D12Device.h"

NAMESPACE_XYH_BEGIN

D3D12ContextCommon::D3D12ContextCommon(D3D12Device* device, ED3D12QueueType queueType, bool bIsDefaultContext)
	: m_device(device)
	, m_queueType(queueType)
	, m_bIsDefaultContext(bIsDefaultContext)
{
}

D3D12CommandContextBase::D3D12CommandContextBase(D3D12Adapter* inParentAdapter)
	: D3D12AdapterChild(inParentAdapter)
{
}

D3D12CommandContext::D3D12CommandContext(D3D12Device* inParentDevice, ED3D12QueueType queueType, bool bIsDefaultContext)
	: D3D12ContextCommon(inParentDevice, queueType, bIsDefaultContext)
	, D3D12CommandContextBase(inParentDevice->GetParentAdapter())
	, D3D12DeviceChild(inParentDevice)
{
}

D3D12CommandContext::~D3D12CommandContext()
{
}

NAMESPACE_XYH_END

