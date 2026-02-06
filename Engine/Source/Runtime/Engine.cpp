#include "Engine.h"
#include <string>
#include <Runtime/Function/Render/RenderSystem.h>
#include "Function/GlobalContext.h"

NAMESPACE_XYH_BEGIN

void XYHEngine::StartEngine(const std::string& configFilePath)
{
	// 初始化所有系统
	g_runtimeGlobalContext.InitSystems(configFilePath);

}

void XYHEngine::ShutdownEngine()
{
	// 释放所有系统
	g_runtimeGlobalContext.ShutdownSystems();
}

void XYHEngine::Inititalize()
{
}

void XYHEngine::Clear()
{
}

bool XYHEngine::IsQuit() const
{
	return m_isQuit;
}

void XYHEngine::Run()
{
}

void XYHEngine::TickOneFrame()
{

	//数据交换
	g_runtimeGlobalContext.m_pRenderSystem->SwapLogicRenderData();

	// 执行渲染

}

int XYHEngine::GetFPS()
{
	return 0;
}

NAMESPACE_XYH_END

