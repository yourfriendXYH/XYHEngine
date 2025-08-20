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
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	// «∞œÚ‰÷»æ
	void DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	// —”≥Ÿ‰÷»æ
	void Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	void SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass);

	RHICommandBuffer* GetRenderCommandBuffer();

private:
	std::shared_ptr<ParticlePass> m_pParticlePass;
};

NAMESPACE_XYH_END