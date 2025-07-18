#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"

NAMESPACE_XYH_BEGIN

RenderSystem::~RenderSystem()
{
}

void RenderSystem::Initialize(ST_RenderSystemInitInfo initInfo)
{
	std::shared_ptr<ConfigManager> pConfigManager = g_runtimeGlobalContext.m_pConfigManager;
}

void RenderSystem::Tick(float deltaTime)
{
}

void RenderSystem::Clear()
{
}

NAMESPACE_XYH_END

