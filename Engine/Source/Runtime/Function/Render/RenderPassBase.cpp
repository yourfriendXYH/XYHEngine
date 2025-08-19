#include "RenderPassBase.h"

NAMESPACE_XYH_BEGIN

void RenderPassBase::PostInitialize()
{
}

void RenderPassBase::SetCommonInfo(const ST_RenderPassCommonInfo& commonInfo)
{
	m_pRHI = commonInfo.m_pRHI;  // 设置渲染接口
	m_pRenderResource = commonInfo.m_pRenderResource;  // 设置渲染资源
}

void RenderPassBase::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
{
}

NAMESPACE_XYH_END

