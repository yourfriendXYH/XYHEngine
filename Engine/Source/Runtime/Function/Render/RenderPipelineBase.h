#pragma once
#include <Common.h>
#include "RenderPassBase.h"
#include "Runtime/Core/Math/Vector2.h"

NAMESPACE_XYH_BEGIN

class RenderResourceBase;
class RHI;

// ��Ⱦ���߳�ʼ����Ϣ
struct RenderPipelineInitInfo
{
	bool m_enableFXAA = false;	// �Ƿ�����FXAA, FXAA ��һ�ָ�Ч����Ļ�ռ俹����㷨���ܹ������ܿ�����С�������������������
	std::shared_ptr<RenderResourceBase> m_renderResource = nullptr;	// ��Ⱦ��Դ
};

class RenderPipelineBase
{
	friend class RenderSystem;	// ��Ⱦϵͳ���Է���˽�г�Ա
public:
	virtual ~RenderPipelineBase() {};

	virtual void Clear() {};	// �����Ⱦ��������
	virtual void Initialize(RenderPipelineInitInfo initInfo) = 0;	// ��ʼ����Ⱦ����

	virtual void PreparePassData(std::shared_ptr<RenderResourceBase> pRenderResource);	// ׼����Ⱦͨ������

	virtual void ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource);	// ǰ����Ⱦ
	virtual void DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource);	// �ӳ���Ⱦ

	//void InitializeUIRenderBackend(WindowUI* pWindowUI);
	virtual uint32_t GetGuidOfPickedMesh(const Vector2& pickedUV) = 0;	// ��ȡʰȡ�������GUID

protected:
	std::shared_ptr<RHI> m_pRHI = nullptr;	// ��ȾӲ���ӿ�

	std::shared_ptr<RenderPassBase> m_pDirectionalLightPass = nullptr;	// ƽ�й���Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pPointLightShadowPass = nullptr;	// ���Դ��Ӱ��Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pMainCameraPass = nullptr;	// ���������Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pColorGradingPass = nullptr;	// ɫ�ʷּ���Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pFxaaPass = nullptr;	// FXAA ��Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pToneMappingPass = nullptr;	// ɫ��ӳ����Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pUIPass = nullptr;	// UI ��Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pCombineUIPass = nullptr;	// �ϲ�UI��Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pPickPass = nullptr;	// ʰȡ��Ⱦͨ��
	std::shared_ptr<RenderPassBase> m_pParticlePass = nullptr;	// ������Ⱦͨ��
};

NAMESPACE_XYH_END