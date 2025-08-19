#pragma once
#include <Common.h>
#include "RenderPassBase.h"
#include "Runtime/Core/Math/Vector2.h"

NAMESPACE_XYH_BEGIN

class RenderResourceBase;
class RHI;

// 渲染管线初始化信息
struct RenderPipelineInitInfo
{
	bool m_enableFXAA = false;	// 是否启用FXAA, FXAA 是一种高效的屏幕空间抗锯齿算法，能够在性能开销较小的情况下提升画面质量
	std::shared_ptr<RenderResourceBase> m_renderResource = nullptr;	// 渲染资源
};

class RenderPipelineBase
{
	friend class RenderSystem;	// 渲染系统可以访问私有成员
public:
	virtual ~RenderPipelineBase() {};

	virtual void Clear() {};	// 清除渲染管线数据
	virtual void Initialize(RenderPipelineInitInfo initInfo) = 0;	// 初始化渲染管线

	virtual void PreparePassData(std::shared_ptr<RenderResourceBase> pRenderResource);	// 准备渲染通道数据

	virtual void ForwardRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource);	// 前向渲染
	virtual void DeferredRender(std::shared_ptr<RHI> pRHI, std::shared_ptr<RenderResourceBase> pRenderResource);	// 延迟渲染

	//void InitializeUIRenderBackend(WindowUI* pWindowUI);
	virtual uint32_t GetGuidOfPickedMesh(const Vector2& pickedUV) = 0;	// 获取拾取的网格的GUID

protected:
	std::shared_ptr<RHI> m_pRHI = nullptr;	// 渲染硬件接口

	std::shared_ptr<RenderPassBase> m_pDirectionalLightPass = nullptr;	// 平行光渲染通道
	std::shared_ptr<RenderPassBase> m_pPointLightShadowPass = nullptr;	// 点光源阴影渲染通道
	std::shared_ptr<RenderPassBase> m_pMainCameraPass = nullptr;	// 主摄像机渲染通道
	std::shared_ptr<RenderPassBase> m_pColorGradingPass = nullptr;	// 色彩分级渲染通道
	std::shared_ptr<RenderPassBase> m_pFxaaPass = nullptr;	// FXAA 渲染通道
	std::shared_ptr<RenderPassBase> m_pToneMappingPass = nullptr;	// 色调映射渲染通道
	std::shared_ptr<RenderPassBase> m_pUIPass = nullptr;	// UI 渲染通道
	std::shared_ptr<RenderPassBase> m_pCombineUIPass = nullptr;	// 合并UI渲染通道
	std::shared_ptr<RenderPassBase> m_pPickPass = nullptr;	// 拾取渲染通道
	std::shared_ptr<RenderPassBase> m_pParticlePass = nullptr;	// 粒子渲染通道
};

NAMESPACE_XYH_END