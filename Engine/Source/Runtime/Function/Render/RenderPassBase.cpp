#include "RenderPassBase.h"

NAMESPACE_XYH_BEGIN

void RenderPassBase::PostInitialize()
{
}

void RenderPassBase::SetCommonInfo(const ST_RenderPassCommonInfo& commonInfo)
{
	m_pRHI = commonInfo.m_pRHI;  // ������Ⱦ�ӿ�
	m_pRenderResource = commonInfo.m_pRenderResource;  // ������Ⱦ��Դ
}

void RenderPassBase::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
{
}

NAMESPACE_XYH_END

