#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

struct ST_ParticlePassInitInfo : public ST_RenderPassInitInfo
{

};

// 色调映射渲染通道
class ParticlePass : public RenderPass
{
public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

private:

};

NAMESPACE_XYH_END