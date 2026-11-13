#pragma once
#include <Common.h>

NAMESPACE_XYH_BEGIN

class D3D12Adapter;
class D3D12Device;

class D3D12AdapterChild
{
public:
	D3D12AdapterChild(D3D12Adapter* inParentAdapter)
		: m_parentAdapter(inParentAdapter)
	{
	}

	D3D12Adapter* GetParentAdapter() const
	{
		assert(m_parentAdapter != nullptr);
		return m_parentAdapter;
	}

	inline void SetParentAdapter(D3D12Adapter* inParentAdapter)
	{
		assert(m_parentAdapter == nullptr);
		m_parentAdapter = inParentAdapter;
	}

protected:
	D3D12Adapter* m_parentAdapter = nullptr;
};

class D3D12DeviceChild
{
public:
	D3D12DeviceChild(D3D12Device* inParentDevice)
		: m_parentDevice(inParentDevice)
	{
	}

	D3D12Device* GetParentDevice() const
	{
		return m_parentDevice;
	}

	inline void SetParentDevice(D3D12Device* inParentDevice)
	{
		m_parentDevice = inParentDevice;
	}

protected:
	D3D12Device* m_parentDevice = nullptr;
};

NAMESPACE_XYH_END