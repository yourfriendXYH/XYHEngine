#pragma once
#include <Common.h>
#include <Runtime/Function/Render/RenderPass.h>

NAMESPACE_XYH_BEGIN

class TestPass : public RenderPass
{
public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

private:

	void SetupAttachments();	// 设置附件

	void SetupRenderPass();	// 设置渲染通道

	void SetupDescriptorSetLayout();	// 设置描述符集布局

	void SetupPipelines();	// 设置管线

};

NAMESPACE_XYH_END