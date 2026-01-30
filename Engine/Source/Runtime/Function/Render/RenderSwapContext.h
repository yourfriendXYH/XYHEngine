#pragma once
#include <Common.h>
#include <string>
#include "../../Resource/ResourceType/Global/GlobalRendering.h"

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

struct ST_RenderSwapData
{
    std::optional<ST_LevelResourceDesc> m_levelResourceDesc;
    //std::optional<GameObjectResourceDesc>  m_game_object_resource_desc;
    //std::optional<GameObjectResourceDesc>  m_game_object_to_delete;
    //std::optional<CameraSwapData>          m_camera_swap_data;
    //std::optional<ParticleSubmitRequest>   m_particle_submit_request;
    //std::optional<EmitterTickRequest>      m_emitter_tick_request;
    //std::optional<EmitterTransformRequest> m_emitter_transform_request;

    //void addDirtyGameObject(GameObjectDesc&& desc);
    //void addDeleteGameObject(GameObjectDesc&& desc);

    //void addNewParticleEmitter(ParticleEmitterDesc& desc);
    //void addTickParticleEmitter(ParticleEmitterID id);
    //void updateParticleTransform(ParticleEmitterTransformDesc& desc);
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
	uint8_t m_RenderSwapDataIndex = ESwapDataType::RenderSwapDataType;

    ST_RenderSwapData m_swapData[ESwapDataType::SwapDataTypeCount];

};

NAMESPACE_XYH_END