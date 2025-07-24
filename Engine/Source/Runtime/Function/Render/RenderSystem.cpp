#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"

NAMESPACE_XYH_BEGIN

RenderSystem::~RenderSystem()
{
}

void RenderSystem::Initialize(ST_RenderSystemInitInfo initInfo)
{
	std::shared_ptr<ConfigManager> pConfigManager = g_runtimeGlobalContext.m_pConfigManager;

	ST_RHIInitInfo rhiInitInfo;
	rhiInitInfo.m_pWindowSystem = initInfo.m_pWindowSystem;
	m_pRHI = std::make_shared<VulkanRHI>();
	m_pRHI->Initialize(rhiInitInfo);
}

void RenderSystem::Tick(float deltaTime)
{
}

void RenderSystem::Clear()
{
}

NAMESPACE_XYH_END

