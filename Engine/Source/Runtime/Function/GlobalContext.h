#pragma once
#include <memory>
#include <string>
#include <Common.h>

NAMESPACE_XYH_BEGIN

class RuntimeGlobalContext
{
public:
	// 创建和初始化所有系统
	void InitSystems(const std::string& configFilePath);
	//清理所有系统
	void ShutdownSystems();

public:
	std::shared_ptr<class LogSystem> m_pLogSystem;	// 日志系统

	std::shared_ptr<class ConfigManager> m_pConfigManager; // 配置管理器

	std::shared_ptr<class RenderSystem> m_pRenderSystem; // 渲染系统
};

extern RuntimeGlobalContext g_runtimeGlobalContext;

NAMESPACE_XYH_END