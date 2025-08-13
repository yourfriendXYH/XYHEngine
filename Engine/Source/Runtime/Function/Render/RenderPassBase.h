#pragma once
#include <Common.h>
#include "Runtime/Function/Render/Interface/RHI.h"

NAMESPACE_XYH_BEGIN

class RenderResourceBase;

struct ST_RenderPassInitInfo
{
};

struct ST_RenderPassCommonInfo
{
	std::shared_ptr<RHI> m_pRHI;  // 渲染硬件接口
	std::shared_ptr<RenderResourceBase> m_pRenderResource;  // 渲染资源
};

class RenderPassBase
{
public:
    virtual void Initialize(const ST_RenderPassInitInfo* initInfo) = 0;
	virtual void PostInitialize();  // 后初始化
	virtual void SetCommonInfo(ST_RenderPassCommonInfo commonInfo);    // 设置通用信息
	virtual void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource);	// 准备渲染通道数据
    //virtual void initializeUIRenderBackend(WindowUI* window_ui);

protected:
	std::shared_ptr<RHI> m_pRHI;  // 渲染硬件接口
	std::shared_ptr<RenderResourceBase> m_pRenderResource;  // 渲染资源
};

NAMESPACE_XYH_END