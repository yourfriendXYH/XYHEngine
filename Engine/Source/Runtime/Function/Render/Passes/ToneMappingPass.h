#pragma once
#include <Common.h>
#include "Runtime/Function/Render/RenderPass.h"

NAMESPACE_XYH_BEGIN

struct ST_ToneMappingPassInitInfo : ST_RenderPassInitInfo
{
    RHIRenderPass* m_pRHIRenderPass;
    RHIImageView* m_pRHIImageView;
};

// ɫ��ӳ����Ⱦͨ��
class ToneMappingPass : public RenderPass
{
public:

private:

};

NAMESPACE_XYH_END