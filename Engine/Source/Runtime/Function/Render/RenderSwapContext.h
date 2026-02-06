#pragma once
#include <Common.h>
#include <string>
#include <optional>
#include <deque>
#include "../../Resource/ResourceType/Global/GlobalRendering.h"
#include "RenderObject.h"
#include "RenderCamera.h"
#include <Runtime/Function/Particle/ParticleDesc.h>

NAMESPACE_XYH_BEGIN

/// 关卡IBL资源描述
struct ST_LevelIBLResourceDesc
{
	SkyBoxIrradianceMap m_skyboxIrradianceMap;  // 天空盒辐照度贴图
	SkyBoxSpecularMap m_skyboxSpecularMap;  // 天空盒高光贴图
	std::string m_brdfMap;	// BRDF贴图
};

/// 关卡颜色分级资源描述
struct ST_LevelColorGradingResourceDesc
{
	std::string m_colorGradingMap;	// 颜色分级贴图
};

// 关卡资源描述
struct ST_LevelResourceDesc
{
	ST_LevelIBLResourceDesc m_iblResourceDesc;	// IBL资源描述
	ST_LevelColorGradingResourceDesc m_colorGradingResourceDesc;	// 颜色分级资源描述
};

// 游戏对象资源描述
struct ST_GameObjectResourceDesc
{
    void Add(GameObjectDesc& desc)
    {
        m_gameObjectDescs.push_back(desc);
    }
    void Pop()
    {
        m_gameObjectDescs.pop_front();
    }

    bool IsEmpty() const
    {
        m_gameObjectDescs.empty();
    }

    GameObjectDesc& GetNextProcessObject()
    {
        m_gameObjectDescs.front();
    }

    // 双端队列
    std::deque<GameObjectDesc> m_gameObjectDescs;
};

struct ST_CameraSwapData
{
    std::optional<float> m_fovX;
    std::optional<ERenderCameraType> m_cameraType;
    std::optional<Matrix4x4> m_viewMatrix;
};

struct ST_ParticleSubmitRequest
{
    void Add(ST_ParticleEmitterDesc& desc)
    {
        m_emitterDescs.push_back(desc);
    }

    unsigned int GetEmitterCount() const
    {
        m_emitterDescs.size();
    }

    const ST_ParticleEmitterDesc& GetEmitterDesc(unsigned int index)
    {
        return m_emitterDescs.at(index);
    }

    std::vector<ST_ParticleEmitterDesc> m_emitterDescs;
};

struct ST_EmitterTickRequest
{
    std::vector<ParticleEmitterID> m_emitterIndices;
};

struct ST_EmitterTransformRequest
{
public:
    void Add(ST_ParticleEmitterTransformDesc& desc)
    {
        m_transformDescs.push_back(desc);
    }

    void Clear()
    {
        m_transformDescs.clear();
    }

    unsigned int GetEmitterCount() const
    {
        return m_transformDescs.size();
    }

    const ST_ParticleEmitterTransformDesc& GetNextEmitterTransformDesc(unsigned int index)
    {
        return m_transformDescs.at(index);
    }

public:
    std::vector<ST_ParticleEmitterTransformDesc> m_transformDescs;
};

struct ST_RenderSwapData
{
public:
    void AddDirtyGameObject(GameObjectDesc&& desc);
    void AddDeleteGameObject(GameObjectDesc&& desc);

    void AddNewParticleEmitter(ST_ParticleEmitterDesc& desc);
    void AddTickParticleEmitter(ParticleEmitterID id);
    void UpdateParticleTransform(ST_ParticleEmitterTransformDesc& desc);

public:
    std::optional<ST_LevelResourceDesc> m_levelResourceDesc;    // 关卡资源

    std::optional<ST_GameObjectResourceDesc> m_gameObjectResourceDesc;
    std::optional<ST_GameObjectResourceDesc> m_gameObjectToDelete;

    std::optional<ST_CameraSwapData> m_cameraSwapData;

    std::optional<ST_ParticleSubmitRequest> m_particleSubmitRequest;
    std::optional<ST_EmitterTickRequest> m_emitterTickRequest;
    std::optional<ST_EmitterTransformRequest> m_emitterTransformRequest;
};

enum ESwapDataType : uint8_t
{
	LogicSwapDataType = 0,
	RenderSwapDataType,
	SwapDataTypeCount
};

class RenderSwapContext
{
public:

    ST_RenderSwapData& GetLogicSwapData();

    ST_RenderSwapData& GetRenderSwapData();

    void SwapLogicRenderData();

    void ResetLevelRsourceSwapData();

    void ResetGameObjectResourceSwapData();

    void ResetGameObjectToDelete();

    void ResetCameraSwapData();

    void ResetPartilceBatchSwapData();

    void ResetEmitterTickSwapData();

    void ResetEmitterTransformSwapData();

private:

    bool IsReadyToSwap() const;

    void Swap();

private:
	uint8_t m_logicSwapDataIndex = ESwapDataType::LogicSwapDataType;
	uint8_t m_renderSwapDataIndex = ESwapDataType::RenderSwapDataType;

    ST_RenderSwapData m_swapData[ESwapDataType::SwapDataTypeCount];

};

NAMESPACE_XYH_END