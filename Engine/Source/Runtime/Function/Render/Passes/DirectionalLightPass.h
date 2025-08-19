#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

class DirectionalLightShadowPass : public RenderPass
{
public:
	void PostInitialize() override final;

private:

};

NAMESPACE_XYH_END