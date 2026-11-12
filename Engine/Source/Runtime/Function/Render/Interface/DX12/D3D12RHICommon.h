#pragma once
#include <Common.h>
#include "D3D12Adapter.h"

NAMESPACE_XYH_BEGIN

class D3D12AdapterChild
{
public:
	D3D12AdapterChild(D3D12Adapter* inParentAdapter)
		:
		m_parentAdapter(inParentAdapter)
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

NAMESPACE_XYH_END