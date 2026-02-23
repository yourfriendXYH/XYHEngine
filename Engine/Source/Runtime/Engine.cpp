#include "Engine.h"
#include <string>
#include <Runtime/Function/Render/RenderSystem.h>
#include <Runtime/Function/Render/WindowSystem.h>
#include "Function/GlobalContext.h"

NAMESPACE_XYH_BEGIN

const float XYHEngine::s_fpsAlpha = 1.0f / 100.0f;

void XYHEngine::StartEngine(const std::string& configFilePath)
{
	// 反射绑定
	// ？？？

	// 初始化所有系统
	g_runtimeGlobalContext.InitSystems(configFilePath);

}

void XYHEngine::ShutdownEngine()
{
	// 释放所有系统
	g_runtimeGlobalContext.ShutdownSystems();

	// 反射绑定结束
	// ？？？
}

void XYHEngine::Inititalize()
{
	// 暂无
}

void XYHEngine::Clear()
{
	// 暂无
}

bool XYHEngine::IsQuit() const
{
	return m_isQuit;
}

void XYHEngine::Run()
{
	// 暂无
	// 目前走的编辑器Run
}

bool XYHEngine::TickOneFrame(float deltaTime)
{
	// 逻辑帧
	LogicalTick(deltaTime);	// 暂时不管

	// 帧率计算
	CalculateFPS(deltaTime);

	//数据交换，逻辑数据变为渲染数据
	g_runtimeGlobalContext.m_pRenderSystem->SwapLogicRenderData();

	// 执行渲染帧
	RendererTick(deltaTime);
	// 

	// 处理窗口事件
	g_runtimeGlobalContext.m_pWindowSystem->PollEvents();

	// 将帧率显示在窗口标题上
	g_runtimeGlobalContext.m_pWindowSystem->SetTitle(std::string("XYH Engine - " + std::to_string(GetFPS()) + " FPS").c_str());

	// 检查窗口是否应该关闭
	bool shouldWindowClose = g_runtimeGlobalContext.m_pWindowSystem->ShouldClose();
	return !shouldWindowClose;
}

int XYHEngine::GetFPS()
{
	return m_fps;
}

void XYHEngine::LogicalTick(float deltaTime)
{
}

void XYHEngine::CalculateFPS(float deltaTime)
{
	// 滑动平均算法
	// 使帧率显示平滑过度
	++m_frameCount;

	if (m_frameCount == 1)
	{
		m_averageDuration = deltaTime;
	}
	else
	{
		// 平滑显示，新帧的权重较低
		m_averageDuration = m_averageDuration * (1.0f - s_fpsAlpha) + deltaTime * s_fpsAlpha;
	}

	m_fps = static_cast<int>(1.0 / m_averageDuration);
}

bool XYHEngine::RendererTick(float deltaTime)
{
	// 执行渲染帧
	g_runtimeGlobalContext.m_pRenderSystem->Tick(deltaTime);
	return true;
}

float XYHEngine::CalculateDeltaTime()
{
	return 0.0f;
}

NAMESPACE_XYH_END

