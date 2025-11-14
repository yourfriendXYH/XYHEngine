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
    // 分配
    RHIDeviceMemory* pDeviceMemory;
    m_pRHI->CreateBuffer(
        sizeof(m_particleCollisionPerframeStorageBufferObject),
        RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_pSceneUniformBuffer,
        pDeviceMemory);
    // 写值
    if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceMemory, 0, RHI_WHOLE_SIZE, 0, &m_pSceneUniformBufferMapped))
    {
        throw std::runtime_error("map billboard uniform buffer");
    }

    RHIDeviceMemory* pDeviceUniformMemory;
    m_pRHI->CreateBufferAndInitialize(
        RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_pComputeUniformBuffer,
        pDeviceUniformMemory,
        sizeof(m_ubo));
    if (RHI_SUCCESS != m_pRHI->MapMemory(pDeviceUniformMemory, 0, RHI_WHOLE_SIZE, 0, &m_pParticleComputeBufferMapped))
    {
        throw std::runtime_error("map buffer");
    }

    //const GlobalParticleRes& global_res = m_particle_manager->getGlobalParticleRes();
    //m_ubo.emit_gap = global_res.m_emit_gap;
    //m_ubo.time_step = global_res.m_time_step;
    //m_ubo.max_life = global_res.m_max_life;
    //m_ubo.gravity = global_res.m_gravity;
    //std::random_device r;
    //std::seed_seq      seed{ r() };
    //m_random_engine.seed(seed);
    //float rnd0 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //float rnd1 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //float rnd2 = m_random_engine.uniformDistribution<float>(0, 1000) * 0.001f;
    //m_ubo.pack = Vector4{ rnd0, static_cast<float>(m_rhi->getCurrentFrameIndex()), rnd1, rnd2 };
    //m_ubo.xemit_count = 100000;

    //m_viewport_params = *m_rhi->getSwapchainInfo().viewport;
    //m_ubo.viewport.x = m_viewport_params.x;
    //m_ubo.viewport.y = m_viewport_params.y;
    //m_ubo.viewport.z = m_viewport_params.width;
    //m_ubo.viewport.w = m_viewport_params.height;
    //m_ubo.extent.x = m_rhi->getSwapchainInfo().scissor->extent.width;
    //m_ubo.extent.y = m_rhi->getSwapchainInfo().scissor->extent.height;

    //memcpy(m_particle_compute_buffer_mapped, &m_ubo, sizeof(m_ubo));

    //{
    //    RHIDeviceMemory* d_mem;
    //    m_rhi->createBuffer(sizeof(m_particlebillboard_perframe_storage_buffer_object),
    //        RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //        RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //        m_particle_billboard_uniform_buffer,
    //        d_mem);

    //    if (RHI_SUCCESS !=
    //        m_rhi->mapMemory(d_mem, 0, RHI_WHOLE_SIZE, 0, &m_particle_billboard_uniform_buffer_mapped))
    //    {
    //        throw std::runtime_error("map billboard uniform buffer");
    //    }
    //}
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

