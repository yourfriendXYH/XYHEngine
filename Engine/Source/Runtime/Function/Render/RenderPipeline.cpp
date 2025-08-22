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
	passCommonInfo.m_pRHI = m_pRHI;	// ������ȾӲ���ӿ�
	passCommonInfo.m_pRenderResource = initInfo.m_renderResource;	// ������Ⱦ��Դ
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
	pParticlePass->Initialize(&particlePassInitInfo);	// ��ʼ��������Ⱦͨ��

	// ������Ӱ��ImageView
	//main_camera_pass->m_point_light_shadow_color_image_view =
	//	std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];
	//main_camera_pass->m_directional_light_shadow_color_image_view =
	//	std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;

	ST_MainCameraPassInitInfp mainCameraPassInitInfo;
	mainCameraPassInitInfo.m_enableFXAA = initInfo.m_enableFXAA;	// �����Ƿ�����FXAA
	pMainCameraPass->SetParticlePass(pParticlePass);	// ����������Ⱦͨ��
	m_pMainCameraPass->Initialize(&mainCameraPassInitInfo);	// ��ʼ�����������Ⱦͨ��

	m_pPointLightShadowPass->PostInitialize();	// ���ʼ�����Դ��Ӱ��Ⱦͨ��
	m_pDirectionalLightPass->PostInitialize();	// ���ʼ��ƽ�й���Ⱦͨ��

	ST_ToneMappingPassInitInfo toneMappingInitInfo;
	//tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
	//tone_mapping_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
	m_pToneMappingPass->Initialize(&toneMappingInitInfo);	// ��ʼ��ɫ��ӳ����Ⱦͨ��

	ST_ColorGradingPassInitInfo colorGradingInitInfo;
	//color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
	//color_grading_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
	m_pColorGradingPass->Initialize(&colorGradingInitInfo);	// ��ʼ��ɫ�ʷּ���Ⱦͨ��

	ST_UIPassInitInfo uiPassInitInfo;
	//ui_init_info.render_pass = _main_camera_pass->getRenderPass();
	m_pUIPass->Initialize(&uiPassInitInfo);	// ��ʼ��UI��Ⱦͨ��

	ST_CombineUIPassInitInfo combineUIPassInitInfo;
	//combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
	//combine_ui_init_info.scene_input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
	//combine_ui_init_info.ui_input_attachment =_main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
	m_pCombineUIPass->Initialize(&combineUIPassInitInfo);	// ��ʼ���ϲ�UI��Ⱦͨ��

	ST_PickPassInitInfo pickPassInitInfo;
	//pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
	m_pPickPass->Initialize(&pickPassInitInfo);	// ��ʼ��ʰȡ��Ⱦͨ��

	ST_FXAAPassInitInfo fxaaPassInitInfo;
	//fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
	//fxaa_init_info.input_attachment = _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd];
	m_pFxaaPass->Initialize(&fxaaPassInitInfo);	// ��ʼ��FXAA��Ⱦͨ��
}

void RenderPipeline::ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI.get());
	RenderResource* pVulkanResource = static_cast<RenderResource*>(pRenderResource.get());

	pVulkanResource->ResetRingBufferOffset(pVulkanRHI->GetCurrentFrameIndex());	// ���û��λ�����ƫ��

	// ������
	pVulkanRHI->WaitForFences();
	pVulkanRHI->ResetCommandPool();

	bool recreateSwapchain = pVulkanRHI->PrepareBeforePass(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// ����Ⱦͨ�����º��ؽ�������???
	if (recreateSwapchain)
		return;

	// ������Ӱ
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

	// ǰ����Ⱦ
	pMainCameraPass->DrawForward(colorGradingPass, fxaaPass, toneMappingPass, uiPass, combineUIPass, particlePass, pVulkanRHI->m_currentSwapchainImageIndex);

	// ��Ⱦ����
	//g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

	pVulkanRHI->SubmitRendering(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// �ύ��Ⱦ
	pParticlePassPtr->CopyNormalAndDepthImage();	// ���Ʒ��ߺ����ͼ��
	pParticlePassPtr->Simulate();	// ???
}

void RenderPipeline::DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource)
{
	VulkanRHI* pVulkanRHI = static_cast<VulkanRHI*>(pRHI.get());
	RenderResource* pVulkanResource = static_cast<RenderResource*>(pRenderResource.get());

	pVulkanResource->ResetRingBufferOffset(pVulkanRHI->GetCurrentFrameIndex());	// ���û��λ�����ƫ��

	// ������
	pVulkanRHI->WaitForFences();
	pVulkanRHI->ResetCommandPool();

	bool recreateSwapchain = pVulkanRHI->PrepareBeforePass(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// ����Ⱦͨ�����º��ؽ�������???
	if (recreateSwapchain)
		return;

	// ������Ӱ
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

	// �ӳ���Ⱦ
	pMainCameraPass->Draw(colorGradingPass, fxaaPass, toneMappingPass, uiPass, combineUIPass, particlePass, pVulkanRHI->m_currentSwapchainImageIndex);

	// ��Ⱦ����
	//g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

	pVulkanRHI->SubmitRendering(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));	// �ύ��Ⱦ
	pParticlePassPtr->CopyNormalAndDepthImage();	// ���Ʒ��ߺ����ͼ��
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

	pickPass.RecreateFramebuffer();	// �ؽ�ʰȡ��Ⱦͨ����֡����
}

uint32_t RenderPipeline::GetGuidOfPickedMesh(const Vector2& pickedUV)
{
	// ��Ļ�ռ�ʰȡ
	// �������ص��ȡ�����GUID
	PickPass& pickPass = *(static_cast<PickPass*>(m_pPickPass.get()));
	return pickPass.Pick(pickedUV);
}

void RenderPipeline::SetAxisVisibleState(bool state)
{
	MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_pMainCameraPass.get()));
	mainCameraPass.m_isShowAxis = state;	// �����Ƿ���ʾ������
}

void RenderPipeline::SetSelectedAxis(size_t selectedAxis)
{
	MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_pMainCameraPass.get()));
	mainCameraPass.m_selectedAxis = selectedAxis;	// ����ѡ�е�������
}

NAMESPACE_XYH_END


