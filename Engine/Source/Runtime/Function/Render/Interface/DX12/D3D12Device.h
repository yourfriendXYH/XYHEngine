#pragma once
#include <Common.h>
#include <d3d12.h>
#include <stack>
#include <array>
#include "D3D12RHICommon.h"
#include "D3D12Queue.h"
#include "D3D12CommandContext.h"

NAMESPACE_XYH_BEGIN

class D3D12Adapter;
class D3D12ContextCommon;
class D3D12CommandContext;
class D3D12AdapterChild;

class D3D12Queue final
{
public:
	struct
	{
		std::stack<D3D12ContextCommon*> contexts;

		
	}m_objectPool;

};

class D3D12Device : public D3D12AdapterChild
{
public:
	D3D12Device(/*FRHIGPUMask InGPUMask,*/ D3D12Adapter* InAdapter);
	~D3D12Device();

	ID3D12Device* GetDevice();

	D3D12ContextCommon* ObtainContext(ED3D12QueueType queueType);
	D3D12CommandContext* ObtainContextCompute()
	{
		return static_cast<D3D12CommandContext*>(ObtainContext(ED3D12QueueType::Async));
	}
	D3D12CommandContext* ObtainContextGraphics()
	{
		return static_cast<D3D12CommandContext*>(ObtainContext(ED3D12QueueType::Direct));
	}

	// Queues
	D3D12Queue& GetQueue(ED3D12QueueType QueueType) { return m_queues[(uint32_t)QueueType]; }
	std::array<D3D12Queue, (uint32_t)ED3D12QueueType::Count> GetQueues() { return m_queues; }

private:
	std::array<D3D12Queue, (uint32_t)ED3D12QueueType::Count> m_queues;
};

NAMESPACE_XYH_END