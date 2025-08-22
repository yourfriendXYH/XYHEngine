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
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"

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
	//particle_init_info.m_particle_manager = g_runtime_global_context.m_particle_manager;
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

	ST_ColorGradingPassInitInfo colorGradingInitInfo;
	//color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
	//color_grading_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
	m_pColorGradingPass->Initialize(&colorGradingInitInfo);	// 初始化色彩分级渲染通道

	ST_UIPassInitInfo uiPassInitInfo;
	//ui_init_info.render_pass = _main_camera_pass->getRenderPass();
	m_pUIPass->Initialize(&uiPassInitInfo);	// 初始化UI渲染通道

	ST_CombineUIPassInitInfo combineUIPassInitInfo;
	//combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
	//combine_ui_init_info.scene_input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
	//combine_ui_init_info.ui_input_attachment =_main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
	m_pCombineUIPass->Initialize(&combineUIPassInitInfo);	// 初始化合并UI渲染通道

	ST_PickPassInitInfo pickPassInitInfo;
	//pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
	m_pPickPass->Initialize(&pickPassInitInfo);	// 初始化拾取渲染通道

	ST_FXAAPassInitInfo fxaaPassInitInfo;
	//fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
	//fxaa_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd];
	m_pFxaaPass->Initialize(&fxaaPassInitInfo);	// 初始化FXAA渲染通道
}

void RenderPipeline::ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI.get());
	RenderResource* pVulkanResource = static_cast<RenderResource*>(pRenderResource.get());

	pVulkanResource->ResetRingBufferOffset(pVulkanRHI->GetCurrentFrameIndex());	// 重置环形缓冲区偏移

	// ？？？
	pVulkanRHI->WaitForFences();
	pVulkanRHI->ResetCommandPool();

	bool recreateSwapchain = pVulkanRHI->PrepareBeforePass(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// 在渲染通道更新后重建交换链???
	if (recreateSwapchain)
		return;

	// 绘制阴影
	static_cast<DirectionalLightShadowPass*>(m_pDirectionalLightPass.get())->Draw();
	static_cast<PointLightShadowPass*>(m_pPointLightShadowPass.get())->Draw();

	ColorGradingPass& colorGradingPass = *(static_cast<ColorGradingPass*>(m_pColorGradingPass.get()));
	FXAAPass& fxaaPass = *(static_cast<FXAAPass*>(m_pFxaaPass.get()));
	ToneMappingPass& toneMappingPass = *(static_cast<ToneMappingPass*>(m_pToneMappingPass.get()));
	UIPass& uiPass = *(static_cast<UIPass*>(m_pUIPass.get()));
	CombineUIPass& combineUIPass = *(static_cast<CombineUIPass*>(m_pCombineUIPass.get()));
	ParticlePass& particlePass = *(static_cast<ParticlePass*>(m_pParticlePass.get()));

	MainCameraPass* pMainCameraPass = static_cast<MainCameraPass*>(m_pMainCameraPass.get());
	ParticlePass* pParticlePassPtr = static_cast<ParticlePass*>(m_pParticlePass.get());

	// ???
	pParticlePassPtr->SetRenderCommandBufferHandle(pMainCameraPass->GetRenderCommandBuffer());

	// 前向渲染
	pMainCameraPass->DrawForward(colorGradingPass, fxaaPass, toneMappingPass, uiPass, combineUIPass, particlePass, pVulkanRHI->m_currentSwapchainImageIndex);

	// 渲染测试
	//g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

	pVulkanRHI->SubmitRendering(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// 提交渲染
	pParticlePassPtr->CopyNormalAndDepthImage();	// 复制法线和深度图像
	pParticlePassPtr->Simulate();	// ???
}

void RenderPipeline::DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI.get());
	RenderResource* pVulkanResource = static_cast<RenderResource*>(pRenderResource.get());

	pVulkanResource->ResetRingBufferOffset(pVulkanRHI->GetCurrentFrameIndex());	// 重置环形缓冲区偏移

	// ？？？
	pVulkanRHI->WaitForFences();
	pVulkanRHI->ResetCommandPool();

	bool recreateSwapchain = pVulkanRHI->PrepareBeforePass(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// 在渲染通道更新后重建交换链???
	if (recreateSwapchain)
		return;

	// 绘制阴影
	static_cast<DirectionalLightShadowPass*>(m_pDirectionalLightPass.get())->Draw();
	static_cast<PointLightShadowPass*>(m_pPointLightShadowPass.get())->Draw();

	ColorGradingPass& colorGradingPass = *(static_cast<ColorGradingPass*>(m_pColorGradingPass.get()));
	FXAAPass& fxaaPass = *(static_cast<FXAAPass*>(m_pFxaaPass.get()));
	ToneMappingPass& toneMappingPass = *(static_cast<ToneMappingPass*>(m_pToneMappingPass.get()));
	UIPass& uiPass = *(static_cast<UIPass*>(m_pUIPass.get()));
	CombineUIPass& combineUIPass = *(static_cast<CombineUIPass*>(m_pCombineUIPass.get()));
	ParticlePass& particlePass = *(static_cast<ParticlePass*>(m_pParticlePass.get()));

	MainCameraPass* pMainCameraPass = static_cast<MainCameraPass*>(m_pMainCameraPass.get());
	ParticlePass* pParticlePassPtr = static_cast<ParticlePass*>(m_pParticlePass.get());

	// ???
	pParticlePassPtr->SetRenderCommandBufferHandle(pMainCameraPass->GetRenderCommandBuffer());

	// 延迟渲染
	pMainCameraPass->Draw(colorGradingPass, fxaaPass, toneMappingPass, uiPass, combineUIPass, particlePass, pVulkanRHI->m_currentSwapchainImageIndex);

	// 渲染测试
	//g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

	pVulkanRHI->SubmitRendering(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// 提交渲染
	pParticlePassPtr->CopyNormalAndDepthImage();	// 复制法线和深度图像
	pParticlePassPtr->Simulate();	// ???
}

void RenderPipeline::PassUpdateAfterRecreateSwapchain()
{
	MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_pMainCameraPass.get()));
	ColorGradingPass& colorGradingPass = *(static_cast<ColorGradingPass*>(m_pColorGradingPass.get()));
	FXAAPass& fxaaPass = *(static_cast<FXAAPass*>(m_pFxaaPass.get()));
	ToneMappingPass& toneMappingPass = *(static_cast<ToneMappingPass*>(m_pToneMappingPass.get()));
	CombineUIPass& combineUIPass = *(static_cast<CombineUIPass*>(m_pCombineUIPass.get()));
	PickPass& pickPass = *(static_cast<PickPass*>(m_pPickPass.get()));
	ParticlePass& particlePass = *(static_cast<ParticlePass*>(m_pParticlePass.get()));

	pickPass.RecreateFramebuffer();	// 重建拾取渲染通道的帧缓冲
}

uint32_t RenderPipeline::GetGuidOfPickedMesh(const Vector2& pickedUV)
{
	// 屏幕空间拾取
	// 根据像素点获取网格的GUID
	PickPass& pickPass = *(static_cast<PickPass*>(m_pPickPass.get()));
	return pickPass.Pick(pickedUV);
}

void RenderPipeline::SetAxisVisibleState(bool state)
{
	MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_pMainCameraPass.get()));
	mainCameraPass.m_isShowAxis = state;	// 设置是否显示坐标轴
}

void RenderPipeline::SetSelectedAxis(size_t selectedAxis)
{
	MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_pMainCameraPass.get()));
	mainCameraPass.m_selectedAxis = selectedAxis;	// 设置选中的坐标轴
}

NAMESPACE_XYH_END


