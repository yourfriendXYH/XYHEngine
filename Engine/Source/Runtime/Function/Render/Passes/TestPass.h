#pragma once
#include <Common.h>
#include <Runtime/Function/Render/RenderPass.h>

NAMESPACE_XYH_BEGIN

class TestPass : public RenderPass
{
public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void Draw() override;

	// 交换链重新创建时需要处理的操作
	void UpdateAfterFramebufferRecreate();

private:

	void SetupAttachments();	// 设置附件

	void SetupRenderPass();	// 设置渲染通道

	void SetupDescriptorSetLayout();	// 设置描述符集布局

	void SetupPipelines();	// 设置管线

	void SetupSwapchainFramebuffers();	// 设置交换链帧缓冲

	void CreateVertexBuffer();
	void CreateIndexBuffer();

private:

	// 没有使用基类RenderPass中的Framebuffer指针
	std::vector<RHIFramebuffer*> m_swapchainFramebuffers;	// 交换链帧缓冲

	RHIBuffer* m_pVertexBuffer = nullptr;
	VmaAllocation m_vertexBufferAllocation;

	RHIBuffer* m_pIndexBuffer = nullptr;
	VmaAllocation m_indexBufferAllocation;
};

NAMESPACE_XYH_END