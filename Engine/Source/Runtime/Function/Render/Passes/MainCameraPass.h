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
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	// 前向渲染
	void DrawForward(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	// 延迟渲染
	void Draw(ColorGradingPass& colorGradingPass, FXAAPass& fxaaPass, ToneMappingPass& toneMappingPass, UIPass& uiPass, CombineUIPass& combineUIPass, ParticlePass& particlePass, uint32_t currentSwapchainImageIndex);

	void SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass);

	RHICommandBuffer* GetRenderCommandBuffer();
public:
	bool m_isShowAxis = false;  // 是否显示坐标轴

	size_t m_selectedAxis = 3u;  // 选中的坐标轴

private:
	std::shared_ptr<ParticlePass> m_pParticlePass;

};

NAMESPACE_XYH_END