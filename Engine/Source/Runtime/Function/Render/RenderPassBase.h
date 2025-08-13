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
	std::shared_ptr<RHI> m_pRHI;  // ��ȾӲ���ӿ�
	std::shared_ptr<RenderResourceBase> m_pRenderResource;  // ��Ⱦ��Դ
};

class RenderPassBase
{
public:
    virtual void Initialize(const ST_RenderPassInitInfo* initInfo) = 0;
	virtual void PostInitialize();  // ���ʼ��
	virtual void SetCommonInfo(ST_RenderPassCommonInfo commonInfo);    // ����ͨ����Ϣ
	virtual void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource);	// ׼����Ⱦͨ������
    //virtual void initializeUIRenderBackend(WindowUI* window_ui);

protected:
	std::shared_ptr<RHI> m_pRHI;  // ��ȾӲ���ӿ�
	std::shared_ptr<RenderResourceBase> m_pRenderResource;  // ��Ⱦ��Դ
};

NAMESPACE_XYH_END