#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"

NAMESPACE_XYH_BEGIN

RenderSystem::~RenderSystem()
{
	Clear();
}

void RenderSystem::Initialize(ST_RenderSystemInitInfo initInfo)
{
	std::shared_ptr<ConfigManager> pConfigManager = g_runtimeGlobalContext.m_pConfigManager;

	ST_RHIInitInfo rhiInitInfo;
	rhiInitInfo.m_pWindowSystem = initInfo.m_pWindowSystem;
	m_pRHI = std::make_shared<VulkanRHI>();
	m_pRHI->Initialize(rhiInitInfo);

	// 全局渲染资源
	// GlobalRenderingRes globalRenderingRes;
	const std::string& globalRenderingResURL = pConfigManager->GetGlobalRenderingResURL();
	// asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

	// 上传ibl，颜色分级纹理
	ST_LevelResourceDesc levelResourceDesc;	// 关卡资源描述
	//level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
	//level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map = global_rendering_res.m_skybox_specular_map;
	//level_resource_desc.m_ibl_resource_desc.m_brdf_map = global_rendering_res.m_brdf_map;
	//level_resource_desc.m_color_grading_resource_desc.m_color_grading_map = global_rendering_res.m_color_grading_map;

	m_renderResource = std::make_shared<RenderResource>();
	m_renderResource->UploadGlobalRenderResource(m_pRHI, levelResourceDesc);	// 上传全局渲染资源
}

void RenderSystem::Tick(float deltaTime)
{


}

void RenderSystem::Clear()
{
}

NAMESPACE_XYH_END

