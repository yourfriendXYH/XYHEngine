#include "Engine.h"
#include "Function/GlobalContext.h"

NAMESPACE_XYH_BEGIN

void XYHEngine::StartEngine()
{
	// 初始化所有系统
	g_runtimeGlobalContext.InitSystems();

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
}

int XYHEngine::GetFPS()
{
	return 0;
}

NAMESPACE_XYH_END

