#pragma once
#include <Common.h>
#include "RenderPipelineBase.h"

NAMESPACE_XYH_BEGIN

class RenderPipeline : public RenderPipelineBase
{
public:
	virtual void Initialize(RenderPipelineInitInfo initInfo) override final;	// ��ʼ����Ⱦ����

	virtual void ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource) override final;	// ǰ����Ⱦ

	virtual void DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource) override final;	// �ӳ���Ⱦ

	virtual uint32_t GetGuidOfPickedMesh(const Vector2& pickedUV) override final;	// ��ȡʰȡ�������GUID

	void PassUpdateAfterRecreateSwapchain();	// �������ؽ��������Ⱦͨ��

	void SetAxisVisibleState(bool state);	// ����������ɼ�״̬

	void SetSelectedAxis(size_t selectedAxis);	// ����ѡ�е�������
};

NAMESPACE_XYH_END