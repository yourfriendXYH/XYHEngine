#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"
#include "Runtime/Function/Render/Passes/ColorGradingPass.h"
#include "Runtime/Function/Render/Passes/FXAAPass.h"
#include "Runtime/Function/Render/Passes/ToneMappingPass.h"
#include "Runtime/Function/Render/Passes/UIPass.h"
#include "Runtime/Function/Render/Passes/CombineUIPass.h"
#include "Runtime/Function/Render/Passes/ParticlePass.h"

NAMESPACE_XYH_BEGIN

struct ST_MainCameraPassInitInfp : public ST_RenderPassInitInfo
{
	bool m_enableFXAA = false;  // 是否启用FXAA
};

class MainCameraPass : public RenderPass
{
public:
	enum ELayoutType : uint8_t
	{
		_per_mesh = 0,	// 每个网格
		_mesh_global,
		_mesh_per_material,
		_skybox,
		_axis,
		_particle,
		_deferred_lighting,
		_layout_type_count
	};

	enum ERenderPipeLineType : uint8_t
	{
		_render_pipeline_type_mesh_gbuffer = 0,	// 延迟渲染的GBuffer阶段
		_render_pipeline_type_deferred_lighting,	// 延迟渲染的光照计算阶段
		_render_pipeline_type_mesh_lighting,
		_render_pipeline_type_skybox,
		_render_pipeline_type_axis,
		_render_pipeline_type_particle,
		_render_pipeline_type_count
	};

public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource) override final;

	// 前向渲染
	void DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	// 延迟渲染
	void Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	void CopyNormalAndDepthImage();

	void SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass);

	RHICommandBuffer* GetRenderCommandBuffer();

	void UpdateAfterFramebufferRecreate();

private:
	void SetupParticlePass();	// 设置粒子渲染通道

	/// <summary>
	/// 创建帧缓冲中的图像及视图
	/// </summary>
	void SetupAttachments();	// 设置附件

	void SetupRenderPass();	// 设置渲染通道

	void SetupDescriptorSetLayout();	// 设置描述符集布局

	void SetupPipelines();	// 设置管线

	void SetupDescriptorSet();	// 设置描述符集

	void SetupFramebufferDescriptorSet();	// 设置帧缓冲描述符集

	void SetupSwapchainFramebuffers();	// 设置交换链帧缓冲

	void SetupModelGlobalDescriptorSet();	// 设置模型全局描述符集
	void SetupSkyboxDescriptorSet();	// 设置天空盒描述符集
	void SetupAxisDescriptorSet();	// 设置坐标轴描述符集
	void SetupParticleDescriptorSet();	// 设置粒子描述符集
	void SetupGbufferLightingDescriptorSet();	// 设置GBuffer光照描述符集

	void DrawMeshGbuffer();	// 绘制网格GBuffer
	void DrawDeferredLighting();	// 绘制延迟光照
	void DrawMeshLighting();	// 绘制网格光照
	void DrawSkybox();	// 绘制天空盒
	void DrawAxis();	// 绘制坐标轴

public:

	RHIImageView* m_pPointLightShadowColorImageView;
	RHIImageView* m_pDirectionalLightShadowColorImageView;

	bool m_isShowAxis = false;  // 是否显示坐标轴

	size_t m_selectedAxis = 3u;  // 选中的坐标轴

	bool m_enableFXAA = false;	// 是否启用FXAA

private:
	std::shared_ptr<ParticlePass> m_pParticlePass;

};

NAMESPACE_XYH_END