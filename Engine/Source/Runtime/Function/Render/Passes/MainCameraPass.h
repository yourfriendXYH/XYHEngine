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
	bool m_enableFXAA = false;  //  «∑Ò∆Ù”√FXAA
};

class MainCameraPass : public RenderPass
{
public:
	enum ELayoutType : uint8_t
	{
		_per_mesh = 0,
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
		_render_pipeline_type_mesh_gbuffer = 0,	// —”≥Ÿ‰÷»æµƒGBufferΩ◊∂Œ
		_render_pipeline_type_deferred_lighting,	// —”≥Ÿ‰÷»æµƒπ‚’’º∆À„Ω◊∂Œ
		_render_pipeline_type_mesh_lighting,
		_render_pipeline_type_skybox,
		_render_pipeline_type_axis,
		_render_pipeline_type_particle,
		_render_pipeline_type_count
	};

public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource) override final;

	// «∞œÚ‰÷»æ
	void DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	// —”≥Ÿ‰÷»æ
	void Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	void SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass);

	RHICommandBuffer* GetRenderCommandBuffer();

	void UpdateAfterFramebufferRecreate();
public:
	bool m_isShowAxis = false;  //  «∑Òœ‘ æ◊¯±Í÷·

	size_t m_selectedAxis = 3u;  // —°÷–µƒ◊¯±Í÷·

private:
	std::shared_ptr<ParticlePass> m_pParticlePass;

};

NAMESPACE_XYH_END