#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

struct ST_PickPassInitInfo : public ST_RenderPassInitInfo
{

};

// ɫ��ӳ����Ⱦͨ��
class PickPass : public RenderPass
{
public:

	void RecreateFramebuffer();

private:

};

NAMESPACE_XYH_END