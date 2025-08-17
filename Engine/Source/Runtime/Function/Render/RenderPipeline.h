#pragma once
#include <Common.h>
#include "RenderPipelineBase.h"

NAMESPACE_XYH_BEGIN

class RenderPipeline : public RenderPipelineBase
{
public:
	virtual void Initialize(RenderPipelineInitInfo initInfo) override final;	// 初始化渲染管线

	virtual void ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource) override final;	// 前向渲染

	virtual void DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource) override final;	// 延迟渲染

	virtual uint32_t GetGuidOfPickedMesh(const Vector2& pickedUV) override final;	// 获取拾取的网格的GUID

	void PassUpdateAfterRecreateSwapchain();	// 交换链重建后更新渲染通道

	void SetAxisVisibleState(bool state);	// 设置坐标轴可见状态

	void SetSelectedAxis(size_t selectedAxis);	// 设置选中的坐标轴
};

NAMESPACE_XYH_END