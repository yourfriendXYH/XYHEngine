#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

struct ST_ColorGradingPassInitInfo : public ST_RenderPassInitInfo
{

};

// 色彩分级后处理
class ColorGradingPass : public RenderPass
{
public:
	void UpdateAfterFramebufferRecreate(RHIImageView* inputAttachment);

private:

};

NAMESPACE_XYH_END