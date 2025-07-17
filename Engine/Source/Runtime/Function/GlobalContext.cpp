#include "GlobalContext.h"
#include "../Core/LogSystem.h"

NAMESPACE_XYH_BEGIN

RuntimeGlobalContext g_runtimeGlobalContext;

void RuntimeGlobalContext::InitSystems()
{
	m_pLogSystem = std::make_shared<LogSystem>(); // 日志系统
}

void RuntimeGlobalContext::ShutdownSystems()
{
	m_pLogSystem.reset();
}

NAMESPACE_XYH_END

