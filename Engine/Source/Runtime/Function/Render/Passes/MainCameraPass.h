#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"
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

	void SetParticlePass(std::shared_ptr<ParticlePass> pParticlePass);

private:
	std::shared_ptr<ParticlePass> m_pParticlePass;
};

NAMESPACE_XYH_END