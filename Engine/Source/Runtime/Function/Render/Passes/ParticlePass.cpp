#include "ParticlePass.h"

NAMESPACE_XYH_BEGIN

void ParticlePass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	RenderPass::Initialize(nullptr);

	const ST_ParticlePassInitInfo* pParticlePassInitInfo = static_cast<const ST_ParticlePassInitInfo*>(initInfo);
	assert(pParticlePassInitInfo != nullptr);
	m_pParticleManager = pParticlePassInitInfo->m_pParticleManager;	// 获取粒子管理器
}

void ParticlePass::SetRenderCommandBufferHandle(RHICommandBuffer* commandBuffer)
{
	m_pRenderCommandBuffer = commandBuffer;
}

void ParticlePass::CopyNormalAndDepthImage()
{
}

void ParticlePass::SetDepthAndNormalImage(RHIImage* pDepthImage, RHIImage* pNormalImage)
{
}

void ParticlePass::SetupParticlePass()
{
	PrepareUniformBuffer();

	SetupDescriptorSetLayout();

	SetupPipelines();

	SetupAttachments();

	ST_RHICommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.m_sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.m_pCommandPool = m_pRHI->GetCommandPoor();
	cmdBufAllocateInfo.m_level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.m_commandBufferCount = 1;	// 次级命令缓冲区
	if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, m_pComputeCommandBuffer))
		throw std::runtime_error("alloc compute command buffer");
	if (RHI_SUCCESS != m_pRHI->AllocateCommandBuffers(&cmdBufAllocateInfo, m_pCopyCommandBuffer))
		throw std::runtime_error("alloc copy command buffer");

	ST_RHIFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.m_sType = RHI_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.m_flags = 0;	// 初始状态：未触发
	if (RHI_SUCCESS != m_pRHI->CreateFence(&fenceCreateInfo, m_pFence))
		throw std::runtime_error("create fence");
}

void ParticlePass::Simulate()
{
}

void ParticlePass::SetRenderPassHandle(RHIRenderPass* pRenderPass)
{
}

void ParticlePass::UpdateAfterFramebufferRecreate()
{
}

void ParticlePass::PrepareUniformBuffer()
{
    // 分配 矩阵数据 的内存
    RHIDeviceMemory* pDeviceMemory;
    m_pRHI->CreateBuffer(
        sizeof(m_particleCollisionPerframeStorageBufferObject), // 内存大小
        RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_pSceneUniformBuffer,
        pDeviceMemory);

    if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceMemory, 0, RHI_WHOLE_SIZE, 0, &m_pSceneUniformBufferMapped))    // 映射从偏移量到内存末尾的整个区域
    {
        throw std::runtime_error("map billboard uniform buffer");
    }

    // 分配内存 粒子生成的参数
    RHIDeviceMemory* pDeviceUniformMemory;
    m_pRHI->CreateBufferAndInitialize(
        RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_pComputeUniformBuffer,
        pDeviceUniformMemory,
        sizeof(m_ubo)); // 内存大小
    if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceUniformMemory, 0, RHI_WHOLE_SIZE, 0, &m_pParticleComputeBufferMapped))  // 映射从偏移量到内存末尾的整个区域
    {
        throw std::runtime_error("map buffer");
    }

    // 粒子生成的属性
    const GlobalParticleResource& globalRes = m_pParticleManager->GetGlobalParticleRes();
    m_ubo.m_emitGap = globalRes.m_emitGap;
    m_ubo.m_timeStep = globalRes.m_timeStep;
    m_ubo.m_maxLife = globalRes.m_maxLife;
    m_ubo.m_gravity = globalRes.m_gravity;
    std::random_device r;
    std::seed_seq seed{ r() };
    //m_random_engine.seed(seed);
    //float rnd0 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //float rnd1 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //float rnd2 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //m_ubo.m_pack = Vector4{ rnd0, static_cast<float>(m_pRHI->GetCurrentFrameIndex()), rnd1, rnd2 };
    m_ubo.m_xemitCount = 100000;

    m_viewportParams = *m_pRHI->GetSwapchainInfo().m_pViewport;
    m_ubo.m_viewport.x = static_cast<uint32_t>(m_viewportParams.m_x);
    m_ubo.m_viewport.y = static_cast<uint32_t>(m_viewportParams.m_y);
    m_ubo.m_viewport.z = static_cast<uint32_t>(m_viewportParams.m_width);
    m_ubo.m_viewport.w = static_cast<uint32_t>(m_viewportParams.m_height);
    m_ubo.m_extent.x = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_width);
    m_ubo.m_extent.y = static_cast<float>(m_pRHI->GetSwapchainInfo().m_pScissor->m_extent.m_height);
    // 给内存赋值
    memcpy(m_pParticleComputeBufferMapped, &m_ubo, sizeof(m_ubo));

    {
        RHIDeviceMemory* pDeviceMemory;
        m_pRHI->CreateBuffer(
            sizeof(m_particleBillboardPerframeStorageBufferObject),
            RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_pParticleBillboardUniformBuffer,
            pDeviceMemory);

        if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceMemory, 0, RHI_WHOLE_SIZE, 0, &m_pParticleBillboardUniformBufferMapped))    // 映射从偏移量到内存末尾的整个区域
        {
            throw std::runtime_error("map billboard uniform buffer");
        }
    }
}

void ParticlePass::SetupAttachments()
{

}

void ParticlePass::SetupDescriptorSetLayout()
{

}

void ParticlePass::SetupPipelines()
{

}

NAMESPACE_XYH_END

