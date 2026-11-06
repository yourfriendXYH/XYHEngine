#pragma once
#include <Common.h>
#include <d3d12.h>

NAMESPACE_XYH_BEGIN

struct D3D12ShaderData
{
	ID3DBlob* m_shaderBuffer = nullptr;

	std::vector<unsigned char> m_code;

	D3D12_SHADER_BYTECODE GetShaderBytecode()
	{
		return D3D12_SHADER_BYTECODE{ m_shaderBuffer->GetBufferPointer(), m_shaderBuffer ->GetBufferSize() };
	}
};

NAMESPACE_XYH_END