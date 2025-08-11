#pragma once
#include <memory>
#include <Common.h>

#include "RenderType.h"
#include "RenderSwapContext.h"

NAMESPACE_XYH_BEGIN

class RHI;
class RenderCamera;
class RenderScene;
class RenderResourceBase;
class RenderPipelineBase;

struct ST_RenderSystemInitInfo
{
	std::shared_ptr<class WindowSystem> m_pWindowSystem; // 窗口系统指针
};

class RenderSystem
{
public:
	RenderSystem() = default;
	~RenderSystem();

	void Initialize(ST_RenderSystemInitInfo initInfo);
	void Tick(float deltaTime);
	void Clear();

	void SwapLogicRenderData();	// 交换逻辑渲染数据

	RenderSwapContext& GetSwapContext();	// 获取渲染交换上下文

	std::shared_ptr<RHI> GetRHI() const;	// 获取渲染硬件接口


private:

	ERENDER_PIPELINE_TYPE m_renderPipelineType{ ERENDER_PIPELINE_TYPE::DEFERRED_PIPELINE };	// 渲染管线类型

	RenderSwapContext m_swapContext;	// 渲染交换上下文

	std::shared_ptr<RHI> m_pRHI;	// 渲染硬件接口

	std::shared_ptr<RenderCamera> m_renderCamera;	// 渲染相机

	std::shared_ptr<RenderScene> m_renderScene;	// 渲染场景

	std::shared_ptr<RenderResourceBase> m_renderResource;

	std::shared_ptr<RenderPipelineBase> m_renderPipeline;
};

NAMESPACE_XYH_END