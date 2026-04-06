#include "D3D12RHI.h"
#include <Runtime/Function/Render/WindowSystem.h>
#include <Runtime/Core/Macro.h>

NAMESPACE_XYH_BEGIN

void D3D12RHI::Initialize(ST_RHIInitInfo initInfo)
{
	m_pGLFWwindow = initInfo.m_pWindowSystem->GetWindow();	// 获取窗口
	std::array<int, 2> windowSize = initInfo.m_pWindowSystem->GetWindowSize(); // 获取窗口大小

	// 获取 调试层接口
#ifdef _DEBUG
	ID3D12Debug* pDebugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
	{
		pDebugController->EnableDebugLayer();	// 启用调试层
		m_dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif // _DEBUG

	CreateDXGIFactory();	// 创建DXGI

	CreateDevice();		// 获取adapter 和 创建设备
}

D3D12RHI::~D3D12RHI()
{
}

void D3D12RHI::CreateDXGIFactory()
{
	// 创建DXGI
	HRESULT hResult = CreateDXGIFactory2(m_dxgiFactoryFlag, IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hResult))
	{
		LOG_ERROR("create dxgi factory faild!");
		return;
	}
}

void D3D12RHI::CreateDevice()
{
	HRESULT hResult;
	// 获取设备
	int adapterIndex = 0;
	bool adapterFound = false;
	// 遍历显卡接口
	while (m_dxgiFactory->EnumAdapters1(adapterIndex, &m_pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc{};
		m_pAdapter->GetDesc1(&desc);
		// 跳过软渲染
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		// 查找设备（大于11版本才有compute shader）
		hResult = D3D12CreateDevice(m_pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hResult))
		{
			adapterFound = true;
			break;
		}
		++adapterIndex;
	}

	if (!adapterFound)
	{
		LOG_ERROR("not found adapter!");
		return;
	}

	hResult = D3D12CreateDevice(m_pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
	if (FAILED(hResult))
	{
		LOG_ERROR("create device faild!");
		return;
	}
}

NAMESPACE_XYH_END

