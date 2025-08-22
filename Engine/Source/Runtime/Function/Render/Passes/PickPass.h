#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

struct ST_PickPassInitInfo : public ST_RenderPassInitInfo
{

};

// 色调映射渲染通道
class PickPass : public RenderPass
{
public:

	void RecreateFramebuffer();

	uint32_t Pick(const Vector2& pickedUV);

private:

};

NAMESPACE_XYH_END