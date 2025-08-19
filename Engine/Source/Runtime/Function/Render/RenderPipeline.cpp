#include "RenderPipeline.h"
#include "Passes/PointLightPass.h"
#include "Passes/DirectionalLightPass.h"
#include "Passes/MainCameraPass.h"
#include "Passes/ToneMappingPass.h"
#include "Passes/ColorGradingPass.h"
#include "Passes/UIPass.h"
#include "Passes/CombineUIPass.h"
#include "Passes/PickPass.h"
#include "Passes/FXAAPass.h"
#include "Passes/ParticlePass.h"

NAMESPACE_XYH_BEGIN

void RenderPipeline::Initialize(RenderPipelineInitInfo initInfo)
{
	m_pPointLightShadowPass = std::make_shared<PointLightShadowPass>();
	m_pDirectionalLightPass = std::make_shared<DirectionalLightShadowPass>();
	m_pMainCameraPass = std::make_shared<MainCameraPass>();
	m_pToneMappingPass = std::make_shared<ToneMappingPass>();
	m_pColorGradingPass = std::make_shared<ColorGradingPass>();
	m_pUIPass = std::make_shared<UIPass>();
	m_pCombineUIPass = std::make_shared<CombineUIPass>();
	m_pPickPass = std::make_shared<PickPass>();
	m_pFxaaPass = std::make_shared<FXAAPass>();
	m_pParticlePass = std::make_shared<ParticlePass>();

	ST_RenderPassCommonInfo passCommonInfo;
	passCommonInfo.m_pRHI = m_pRHI;	// 设置渲染硬件接口
	passCommonInfo.m_pRenderResource = initInfo.m_renderResource;	// 设置渲染资源
	m_pPointLightShadowPass->SetCommonInfo(passCommonInfo);
	m_pDirectionalLightPass->SetCommonInfo(passCommonInfo);
	m_pMainCameraPass->SetCommonInfo(passCommonInfo);
	m_pToneMappingPass->SetCommonInfo(passCommonInfo);
	m_pColorGradingPass->SetCommonInfo(passCommonInfo);
	m_pUIPass->SetCommonInfo(passCommonInfo);
	m_pCombineUIPass->SetCommonInfo(passCommonInfo);
	m_pPickPass->SetCommonInfo(passCommonInfo);
	m_pFxaaPass->SetCommonInfo(passCommonInfo);
	m_pParticlePass->SetCommonInfo(passCommonInfo);

	m_pPointLightShadowPass->Initialize(nullptr);
	m_pDirectionalLightPass->Initialize(nullptr);

	std::shared_ptr<MainCameraPass> pMainCameraPass = std::static_pointer_cast<MainCameraPass>(m_pMainCameraPass);
	std::shared_ptr<RenderPass> pRenderPassMainCamera = std::static_pointer_cast<RenderPass>(m_pMainCameraPass);	// ???
	std::shared_ptr<ParticlePass> pParticlePass = std::static_pointer_cast<ParticlePass>(m_pParticlePass);

	ST_ParticlePassInitInfo particlePassInitInfo;

	pParticlePass->Initialize(&particlePassInitInfo);	// 初始化粒子渲染通道

	// 两种阴影的ImageView
	//main_camera_pass->m_point_light_shadow_color_image_view =
	//	std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];
	//main_camera_pass->m_directional_light_shadow_color_image_view =
	//	std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;

	ST_MainCameraPassInitInfp mainCameraPassInitInfo;
	mainCameraPassInitInfo.m_enableFXAA = initInfo.m_enableFXAA;	// 设置是否启用FXAA
	pMainCameraPass->SetParticlePass(pParticlePass);	// 设置粒子渲染通道
	m_pMainCameraPass->Initialize(&mainCameraPassInitInfo);	// 初始化主摄像机渲染通道

	m_pPointLightShadowPass->PostInitialize();	// 后初始化点光源阴影渲染通道
	m_pDirectionalLightPass->PostInitialize();	// 后初始化平行光渲染通道

	ST_ToneMappingPassInitInfo toneMappingInitInfo;
	//tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
	//tone_mapping_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
	m_pToneMappingPass->Initialize(&toneMappingInitInfo);	// 初始化色调映射渲染通道
}

void RenderPipeline::ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

void RenderPipeline::DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
}

uint32_t RenderPipeline::GetGuidOfPickedMesh(const Vector2& pickedUV)
{
	return 0;
}

void RenderPipeline::PassUpdateAfterRecreateSwapchain()
{
}

void RenderPipeline::SetAxisVisibleState(bool state)
{
}

void RenderPipeline::SetSelectedAxis(size_t selectedAxis)
{
}

NAMESPACE_XYH_END


