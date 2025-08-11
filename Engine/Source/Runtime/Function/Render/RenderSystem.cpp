#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"
#include "RenderCamera.h"

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

	// 初始化相机参数
	m_renderCamera = std::make_shared<RenderCamera>();	// 创建渲染相机
	m_renderCamera->LookAt(Vector3(-5.0f, 0.0f, 3.0f), Vector3(-4.0f, 0.0f, 3.0f), Vector3(0.0f, 0.0f, 1.0f));	// 设置相机初始位置和方向
	m_renderCamera->m_zFar = 1000.0f;	// 设置远裁剪面
	m_renderCamera->m_zNear = 0.1f;		// 设置近裁剪面
	m_renderCamera->SetAspectRatio(1280.0f / 768.0f);	// 设置宽高比

}

void RenderSystem::Tick(float deltaTime)
{


}

void RenderSystem::Clear()
{
}

NAMESPACE_XYH_END

