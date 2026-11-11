#include "PreZ.h"
#include <Runtime/Function/Render/RenderPass.h>
#include <Runtime/Function/Render/Interface/DX12/D3D12RHI.h>
#include <Runtime/Function/Render/Interface/DX12/D3D12Util.h>

NAMESPACE_XYH_BEGIN

static RenderPass* sPreZPass = nullptr;

static D3D12_VERTEX_BUFFER_VIEW sVBO;
static D3D12_INDEX_BUFFER_VIEW sIBO;

void InitPreZPass(std::shared_ptr<RHI> inRHI, RHIBuffer* inPositionBuffer, RHIBuffer* inAttributeBuffer, RHIBuffer* inIndexBuffer, RHITexture* inDSRT)
{
    sVBO = {};
    sVBO.BufferLocation = ((D3D12Buffer*)inPositionBuffer)->m_resource->GetGPUVirtualAddress();
    sVBO.StrideInBytes = sizeof(float) * 3;
    sVBO.SizeInBytes = sizeof(float) * 3;

    sIBO = {};
    sIBO.BufferLocation = ((D3D12Buffer*)inIndexBuffer)->m_resource->GetGPUVirtualAddress();
    sIBO.Format = DXGI_FORMAT_R16_UINT;
    sIBO.SizeInBytes = ((D3D12Buffer*)inIndexBuffer)->m_resource->GetDesc().Width;

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
    D3D12RHI* d3d12RHI = dynamic_cast<D3D12RHI*>(inRHI.get());

    sPreZPass->PreExecute();

    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    barriers.push_back(D3D12Util::InitResourceBarrier(((D3D12Texture*)sPreZPass->m_DSRT)->m_underlyingResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
    d3d12RHI->GetGraphicsCommandList()->ResourceBarrier(barriers.size(), barriers.data());
    barriers.clear();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandleDSRT = sPreZPass->m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    d3d12RHI->GetGraphicsCommandList()->OMSetRenderTargets(0, nullptr, FALSE, &cpuDescriptorHandleDSRT);
    d3d12RHI->GetGraphicsCommandList()->ClearDepthStencilView(cpuDescriptorHandleDSRT, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.0f, 0, 0, nullptr); // reverse-Z用0清除深度

    d3d12RHI->GetGraphicsCommandList()->SetGraphicsRootSignature(sPreZPass->m_rootSignature);
    d3d12RHI->GetGraphicsCommandList()->SetPipelineState(sPreZPass->m_PSO);
    d3d12RHI->GetGraphicsCommandList()->RSSetViewports(1, &sPreZPass->m_viewport);
    d3d12RHI->GetGraphicsCommandList()->RSSetScissorRects(1, &sPreZPass->m_scissor);

    for (size_t i = 0; i < sPreZPass->m_rootConstantBuffers.size(); i++)
    {
        BindingCB& cb = sPreZPass->m_rootConstantBuffers[i];
        d3d12RHI->GetGraphicsCommandList()->SetGraphicsRootConstantBufferView(i, ((D3D12Buffer*)cb.m_resource)->m_resource->GetGPUVirtualAddress());
    }

    const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    d3d12RHI->GetGraphicsCommandList()->OMSetBlendFactor(blendFactor);

    d3d12RHI->GetGraphicsCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d12RHI->GetGraphicsCommandList()->IASetVertexBuffers(0, 1, &sVBO);
    d3d12RHI->GetGraphicsCommandList()->IASetIndexBuffer(&sIBO);

    //6 x 4 x 2 x 3 => 48 x 3 = 144
    d3d12RHI->GetGraphicsCommandList()->DrawIndexedInstanced(144, 2, 0, 0, 0);

    barriers.push_back(D3D12Util::InitResourceBarrier(((D3D12Texture*)sPreZPass->m_DSRT)->m_underlyingResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
    d3d12RHI->GetGraphicsCommandList()->ResourceBarrier(barriers.size(), barriers.data());
    d3d12RHI->SubmitRendering(nullptr);
}


NAMESPACE_XYH_END