#pragma once
#include <memory>
#include "../../Common/Common.h"

NAMESPACE_XYH_BEGIN

class RuntimeGlobalContext
{
public:
	// 创建和初始化所有系统
	void InitSystems();
	//清理所有系统
	void ShutdownSystems();

public:
	std::shared_ptr<class LogSystem> m_pLogSystem;


};

extern RuntimeGlobalContext g_runtimeGlobalContext;

NAMESPACE_XYH_END