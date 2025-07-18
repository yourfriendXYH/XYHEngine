#include "GlobalContext.h"
#include "../Core/LogSystem.h"
#include "../Resource/ConfigManager.h"
#include "Render/RenderSystem.h"

NAMESPACE_XYH_BEGIN

RuntimeGlobalContext g_runtimeGlobalContext;

void RuntimeGlobalContext::InitSystems(const std::string& configFilePath)
{
	m_pLogSystem = std::make_shared<LogSystem>(); // 日志系统

	m_pConfigManager = std::make_shared<ConfigManager>(); // 配置管理器
	m_pConfigManager->Initialize(configFilePath); // 初始化配置管理器

	m_pRenderSystem = std::make_shared<RenderSystem>(); // 渲染系统
	ST_RenderSystemInitInfo renderInitInfo; // 渲染系统初始化信息
	renderInitInfo;
	m_pRenderSystem->Initialize(renderInitInfo); // 初始化渲染系统
}

void RuntimeGlobalContext::ShutdownSystems()
{
	m_pLogSystem.reset(); // 清理日志系统

	m_pRenderSystem->Clear(); // 清理渲染系统
	m_pRenderSystem.reset(); // 清理渲染系统指针
}

NAMESPACE_XYH_END

