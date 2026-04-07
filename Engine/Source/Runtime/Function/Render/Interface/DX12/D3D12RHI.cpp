#include "D3D12RHI.h"
#include <Runtime/Function/Render/WindowSystem.h>
#include <Runtime/Core/Macro.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

NAMESPACE_XYH_BEGIN

void D3D12RHI::Initialize(ST_RHIInitInfo initInfo)
{
	m_pGLFWwindow = initInfo.m_pWindowSystem->GetWindow();	// 获取窗口
	std::array<int, 2> windowSize = initInfo.m_pWindowSystem->GetWindowSize(); // 获取窗口大小
	m_viewport = { 0.0f, 0.0f, static_cast<float>(windowSize[0]), static_cast<float>(windowSize[1]), 0.0f, 1.0f }; // 设置视口大小
	m_scissor = { { 0, 0 }, { (uint32_t)windowSize[0], (uint32_t)windowSize[1] } }; // 设置裁剪区域大小

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

	CreateCommandQueue();	// 创建命令队列

	CreateSwapChain();	// 创建交换链
}

D3D12RHI::~D3D12RHI()
{
}

void D3D12RHI::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferCount = s_maxFramesInFlight;	// 缓冲数量
	desc.BufferDesc = {};
	desc.BufferDesc.Width = m_viewport.m_width;
	desc.BufferDesc.Height = m_viewport.m_height;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = glfwGetWin32Window(m_pGLFWwindow);
	desc.SampleDesc.Count = 1;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	IDXGISwapChain* pSwapChain = nullptr;
	HRESULT hResult = m_dxgiFactory->CreateSwapChain(m_pCommandQueue, &desc, &pSwapChain);
	m_pSwapChain = static_cast<IDXGISwapChain3*>(pSwapChain);
	if (FAILED(hResult) || nullptr == m_pSwapChain)
	{
		LOG_ERROR("create swap chain failed");
	}
}

void D3D12RHI::CreateDXGIFactory()
{
	// 创建DXGI
	HRESULT hResult = CreateDXGIFactory2(m_dxgiFactoryFlag, IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hResult))
	{
		LOG_ERROR("create dxgi factory failed!");
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
		LOG_ERROR("create device failed!");
		return;
	}
}

void D3D12RHI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	HRESULT hResult = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
	if (FAILED(hResult))
	{
		LOG_ERROR("create command queue failed!");
	}
}

NAMESPACE_XYH_END

