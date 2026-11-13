#pragma once
#include <Common.h>
#include <d3d12.h>
#include <array>

NAMESPACE_XYH_BEGIN

class D3D12Device;

class D3D12Adapter
{
public:
	D3D12Adapter();
	virtual ~D3D12Adapter();

	ID3D12Device* GetD3DDevice() const { return m_rootDevice; }

protected:
	UINT m_dxgiFactoryFlag = 0u;
	IDXGIFactory4* m_dxgiFactory = nullptr;	// DXGI

	IDXGIAdapter1* m_pAdapter = nullptr;	// от©╗╫с©з

	ID3D12Device* m_rootDevice = nullptr;
	
	std::array<D3D12Device*, 1> m_devices;

private:

};

NAMESPACE_XYH_END