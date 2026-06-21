#include "D3D12RHI.h"
#include <Runtime/Function/Render/WindowSystem.h>
#include <Runtime/Core/Macro.h>

#include "D3D12Util.h"

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

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	D3D12Util::CreateResource(
		m_pDepthStencilRenderTarget,
		m_pDevice,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		m_viewport.m_width,
		m_viewport.m_height,
		0,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		1,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescRTV{};
	descriptorHeapDescRTV.NumDescriptors = s_maxFramesInFlight;
	descriptorHeapDescRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_pDevice->CreateDescriptorHeap(&descriptorHeapDescRTV, IID_PPV_ARGS(&m_pRTVHeap));
	m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescDSV{};
	descriptorHeapDescDSV.NumDescriptors = 1;
	descriptorHeapDescDSV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_pDevice->CreateDescriptorHeap(&descriptorHeapDescDSV, IID_PPV_ARGS(&m_pDSVHeap));
	m_dsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart = m_pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < s_maxFramesInFlight; i++)
	{
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorRenderTarget[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE rtvPointer;
		rtvPointer.ptr = rtvHeapStart.ptr + i * m_rtvDescriptorSize;
		m_pDevice->CreateRenderTargetView(m_pColorRenderTarget[i], nullptr, rtvPointer);
	}
	m_currentRenderTargetIndex = m_pSwapChain->GetCurrentBackBufferIndex();	// 获取当前渲染目标的索引

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	m_pDevice->CreateDepthStencilView(m_pDepthStencilRenderTarget, &depthStencilViewDesc, m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

	m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
	m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pGraphicsCommandList));

	m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
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

void D3D12RHI::WaitForFences()
{
	WaitForCompletionOfCommandList();
}

void D3D12RHI::ResetCommandPool()
{
	m_pCommandAllocator->Reset();
	m_pGraphicsCommandList->Reset(m_pCommandAllocator, nullptr);
}

uint8_t D3D12RHI::GetCurrentFrameIndex() const
{
	return m_currentRenderTargetIndex;
}

void D3D12RHI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
{
	EndCommandList();

	m_pSwapChain->Present(0, 0);
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

void D3D12RHI::WaitForCompletionOfCommandList()
{
	if (m_pFence->GetCompletedValue() < m_fenceValue)	// 等待上一帧指令执行完成
	{
		m_pFence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_currentRenderTargetIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void D3D12RHI::EndCommandList()
{
	m_pGraphicsCommandList->Close();
	std::array<ID3D12CommandList*, 1> pCommandLists = { m_pGraphicsCommandList };
	m_pCommandQueue->ExecuteCommandLists(pCommandLists.size(), pCommandLists.data());

	m_fenceValue += 1;
	m_pCommandQueue->Signal(m_pFence, m_fenceValue);
}

ID3D12PipelineState* D3D12RHI::CreatePSO(ID3D12RootSignature* pRootSignature, D3D12_SHADER_BYTECODE VSByteCode, D3D12_SHADER_BYTECODE PSByteCode)
{
	const size_t elementSize = 4u;
	D3D12_INPUT_ELEMENT_DESC vertexDataElementDesc[elementSize] = {
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC vertexDataLayoutDesc{};
	vertexDataLayoutDesc.NumElements = elementSize;
	vertexDataLayoutDesc.pInputElementDescs = vertexDataElementDesc;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = pRootSignature;
	psoDesc.VS = VSByteCode;
	psoDesc.PS = PSByteCode;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.InputLayout = vertexDataLayoutDesc;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FrontCounterClockwise = false;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;

	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	psoDesc.BlendState = { 0 };
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc{
		FALSE,
		FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (size_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		psoDesc.BlendState.RenderTarget[i] = rtBlendDesc;
	}
	psoDesc.NumRenderTargets = 1;

	ID3D12PipelineState* pPSO = nullptr;
	HRESULT hResult = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPSO));
	if (FAILED(hResult))
	{
		return nullptr;
	}
	return pPSO;
}

void D3D12RHI::CreateShaderFromFile(LPCTSTR shaderFilePath, const char* mainFunctionName, const char* target, D3D12_SHADER_BYTECODE* pShader)
{
	ID3DBlob* pShaderBuffer = nullptr;
	ID3DBlob* pErrorBuffer = nullptr;
	HRESULT hResult = D3DCompileFromFile(shaderFilePath, nullptr, nullptr, mainFunctionName, target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pShaderBuffer, &pErrorBuffer);
	if (FAILED(hResult))
	{
		return;
	}
	pShader->pShaderBytecode = pShaderBuffer->GetBufferPointer();
	pShader->BytecodeLength = pShaderBuffer->GetBufferSize();
}

ID3D12RootSignature* D3D12RHI::InitRootSignature()
{
	const unsigned int parameterSize = 2u;
	D3D12_ROOT_PARAMETER parameters[parameterSize] = {};
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameters[0].Constants.RegisterSpace = 0;
	parameters[0].Constants.ShaderRegister = 0;	// 对应shader中的 b0
	parameters[0].Constants.Num32BitValues = 4;	// 4个float

	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameters[1].Descriptor.RegisterSpace = 0;
	parameters[1].Descriptor.ShaderRegister = 1;	// b1

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = parameterSize;
	rootSignatureDesc.pParameters = parameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// 32bit constants 最多存储64个DWORD -> 64 * 4 = 256字节
	ID3DBlob* signature;
	HRESULT hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);

	ID3D12RootSignature* pRootSignature;
	m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));

	return pRootSignature;
}

void D3D12RHI::BeginRenderToSwapChain(ID3D12GraphicsCommandList* pGraphicsCommandList)
{
	D3D12_RESOURCE_BARRIER barrier = D3D12Util::InitResourceBarrier(m_pColorRenderTarget[m_currentRenderTargetIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGraphicsCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE colorRenderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
	colorRenderTarget.ptr = m_pRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_currentRenderTargetIndex * m_rtvDescriptorSize;
	depthStencilView.ptr = m_pDSVHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	pGraphicsCommandList->OMSetRenderTargets(1, &colorRenderTarget, FALSE, &depthStencilView);

	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, 1280.0f, 720.0f };
	D3D12_RECT scissorRect = { 0.0f, 0.0f, 1280.0f, 720.0f };
	m_pGraphicsCommandList->RSSetViewports(1, &viewport);
	m_pGraphicsCommandList->RSSetScissorRects(1, &scissorRect);

	const float clearColor[] = { 1.0f, 0.4f, 0.6f, 1.0f };
	m_pGraphicsCommandList->ClearRenderTargetView(colorRenderTarget, clearColor, 0, nullptr);
	m_pGraphicsCommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void D3D12RHI::EndRenderToSwapChain(ID3D12GraphicsCommandList* pGraphicsCommandList)
{
	D3D12_RESOURCE_BARRIER barrier = D3D12Util::InitResourceBarrier(m_pColorRenderTarget[m_currentRenderTargetIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	pGraphicsCommandList->ResourceBarrier(1, &barrier);
}

ID3D12GraphicsCommandList* D3D12RHI::GetGraphicsCommandList() const
{
	return m_pGraphicsCommandList;
}

ID3D12Device* D3D12RHI::GetDevice() const
{
	return m_pDevice;
}

NAMESPACE_XYH_END

