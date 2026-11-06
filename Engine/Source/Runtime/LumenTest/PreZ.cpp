#include "PreZ.h"
#include <Runtime/Function/Render/RenderPass.h>
#include <Runtime/Function/Render/Interface/DX12/D3D12RHI.h>

NAMESPACE_XYH_BEGIN

static RenderPass* sPreZPass = nullptr;

void InitPreZPass(std::shared_ptr<RHI> inRHI, RHIBuffer* inPositionBuffer, RHIBuffer* inAttributeBuffer, RHIBuffer* inIndexBuffer, RHITexture* inDSRT)
{
	RenderPass* renderPass = new RenderPass(inRHI, ERenderPassType::ERPT_GRAPHICS, "PreZ");
	RHIBuffer* globalConstantBuffer = nullptr;
	renderPass->AddConstantBuffer(0, globalConstantBuffer, D3D12_SHADER_VISIBILITY_PIXEL, true);
	renderPass->SetDSRT(inDSRT);
	renderPass->InitRootSignature();
	sPreZPass = renderPass;

	D3D12RHI* d3d12RHI = dynamic_cast<D3D12RHI*>(inRHI.get());
	D3D12ShaderData vsShader = d3d12RHI->CreateShaderFromFileSM6(L"Engine/Shader/hlsl/PreZ.hlsl", L"MainVS", L"vs_6_0");

	D3D12_INPUT_ELEMENT_DESC vertexDataElementDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	D3D12_INPUT_LAYOUT_DESC vertexDataLayoutDesc = {};
	vertexDataLayoutDesc.NumElements = _countof(vertexDataElementDesc);
	vertexDataLayoutDesc.pInputElementDescs = vertexDataElementDesc;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = sPreZPass->m_rootSignature;
    psoDesc.VS = vsShader.GetShaderBytecode();
    psoDesc.DSVFormat = ((D3D12Texture*)inDSRT)->m_RTVFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.SampleMask = 0xffffffff;
    psoDesc.InputLayout = vertexDataLayoutDesc;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = true;
    psoDesc.RasterizerState.FrontCounterClockwise = true;

    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

    psoDesc.BlendState = { 0 };
    psoDesc.BlendState.IndependentBlendEnable = true;
    for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        if (i < sPreZPass->m_RTBlendDescs.size())
        {
            psoDesc.BlendState.RenderTarget[i] = sPreZPass->m_RTBlendDescs[i];
        }
        else
        {
            psoDesc.BlendState.RenderTarget[i] = sPreZPass->m_RT0BlendDesc;
        }
    }
    psoDesc.NumRenderTargets = sPreZPass->m_RTs.size();
    HRESULT hResult = d3d12RHI->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&sPreZPass->m_PSO));
    if (FAILED(hResult))
    {
        printf("CreateGraphicsPipelineState failed: 0x%08x\n", hResult);
    }
    
    sPreZPass->m_viewport = { inRHI->m_viewport.m_x, inRHI->m_viewport.m_y, inRHI->m_viewport.m_width, inRHI->m_viewport.m_height, 0.0f, 1.0f };
    sPreZPass->m_scissor = { inRHI->m_scissor.m_offset.m_x, inRHI->m_scissor.m_offset.m_y, (long)inRHI->m_scissor.m_extent.m_width, (long)inRHI->m_scissor.m_extent.m_height };
}

void ExecutePreZPass(std::shared_ptr<RHI> inRHI)
{
}


NAMESPACE_XYH_END