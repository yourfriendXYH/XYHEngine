#include "TestPass.h"
#include <Runtime/Function/Render/Interface/Vulkan/VulkanRHI.h>

NAMESPACE_XYH_BEGIN

void TestPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	SetupPipelines();
}

void TestPass::SetupAttachments()
{

}

void TestPass::SetupPipelines()
{
	m_renderPipelines.resize(1);
	std::vector<unsigned char> vsByteCode = VulkanRHI::ReadShaderByteCode("D:/DirectX12_learn/XYHEngine/Engine/Shader/Generated/spv/testVkVert.spv");
	std::vector<unsigned char> fsByteCode = VulkanRHI::ReadShaderByteCode("D:/DirectX12_learn/XYHEngine/Engine/Shader/Generated/spv/testVkFrag.spv");
	RHIShader* pVertShaderModule = m_pRHI->CreateShaderModule(vsByteCode);
	RHIShader* pFragShaderModule = m_pRHI->CreateShaderModule(fsByteCode);

}

NAMESPACE_XYH_END

