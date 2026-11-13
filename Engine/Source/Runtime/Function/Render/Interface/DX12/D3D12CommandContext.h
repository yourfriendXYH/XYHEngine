#pragma once
#include <Common.h>
#include "D3D12Queue.h"
#include "D3D12RHICommon.h"

NAMESPACE_XYH_BEGIN

class D3D12Device;

class D3D12ContextCommon
{
protected:
	D3D12ContextCommon(D3D12Device* device, ED3D12QueueType queueType, bool bIsDefaultContext);

public:
	virtual ~D3D12ContextCommon() = default;

public:

	D3D12Device* const m_device;

	ED3D12QueueType const m_queueType;

	bool const m_bIsDefaultContext;
};

class D3D12CommandContextBase : public D3D12AdapterChild
{
public:
	D3D12CommandContextBase(D3D12Adapter* inParentAdapter/*, FRHIGPUMask InGPUMask*/);
};

class D3D12CommandContext : public D3D12ContextCommon, public D3D12CommandContextBase, public D3D12DeviceChild
{
public:
	D3D12CommandContext(class D3D12Device* inParentDevice, ED3D12QueueType queueType, bool bIsDefaultContext);
	virtual ~D3D12CommandContext();


};

NAMESPACE_XYH_END