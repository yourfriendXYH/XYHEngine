#include "RenderSwapContext.h"

NAMESPACE_XYH_BEGIN

ST_RenderSwapData& RenderSwapContext::GetLogicSwapData()
{
    m_swapData[m_logicSwapDataIndex];
}

ST_RenderSwapData& RenderSwapContext::GetRenderSwapData()
{
    m_swapData[m_renderSwapDataIndex];
}

void RenderSwapContext::SwapLogicRenderData()
{
    if (IsReadyToSwap())
    {
        // 重置渲染数据，并将逻辑数据和渲染数据的索引交换
        Swap();
    }
}

void RenderSwapContext::ResetLevelRsourceSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_levelResourceDesc.reset();
}

void RenderSwapContext::ResetGameObjectResourceSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_gameObjectResourceDesc.reset();
}

void RenderSwapContext::ResetGameObjectToDelete()
{
    m_swapData[m_renderSwapDataIndex].m_gameObjectToDelete.reset();
}

void RenderSwapContext::ResetCameraSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_cameraSwapData.reset();
}

void RenderSwapContext::ResetPartilceBatchSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_particleSubmitRequest.reset();
}

void RenderSwapContext::ResetEmitterTickSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_emitterTickRequest.reset();
}

void RenderSwapContext::ResetEmitterTransformSwapData()
{
    m_swapData[m_renderSwapDataIndex].m_emitterTransformRequest.reset();
}

bool RenderSwapContext::IsReadyToSwap() const
{
    // 渲染的交换数据都没值时返回true，有一个有值返回false
    return !(m_swapData[m_renderSwapDataIndex].m_levelResourceDesc.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_gameObjectResourceDesc.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_gameObjectToDelete.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_cameraSwapData.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_particleSubmitRequest.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_emitterTickRequest.has_value() || 
            m_swapData[m_renderSwapDataIndex].m_emitterTransformRequest.has_value());
}

void RenderSwapContext::Swap()
{
    ResetLevelRsourceSwapData();
    ResetGameObjectResourceSwapData();
    ResetGameObjectToDelete();
    ResetCameraSwapData();
    ResetPartilceBatchSwapData();
    ResetEmitterTickSwapData();
    ResetEmitterTransformSwapData();

    std::swap(m_logicSwapDataIndex, m_renderSwapDataIndex);
}

void ST_RenderSwapData::AddDirtyGameObject(GameObjectDesc&& desc)
{
    if (m_gameObjectResourceDesc.has_value())
    {
        m_gameObjectResourceDesc->Add(desc);
    }
    else
    {
        ST_GameObjectResourceDesc goDescs;
        goDescs.Add(desc);
        m_gameObjectResourceDesc = goDescs;
    }
}

void ST_RenderSwapData::AddDeleteGameObject(GameObjectDesc&& desc)
{
    if (m_gameObjectToDelete.has_value())
    {
        m_gameObjectToDelete->Add(desc);
    }
    else
    {
        ST_GameObjectResourceDesc goDescs;
        goDescs.Add(desc);
        m_gameObjectToDelete = goDescs;
    }
}

void ST_RenderSwapData::AddNewParticleEmitter(ST_ParticleEmitterDesc& desc)
{
    if (m_particleSubmitRequest.has_value())
    {
        m_particleSubmitRequest->Add(desc);
    }
    else
    {
        ST_ParticleSubmitRequest request;
        request.Add(desc);
        m_particleSubmitRequest = request;
    }
}

void ST_RenderSwapData::AddTickParticleEmitter(ParticleEmitterID id)
{
    if (m_emitterTickRequest.has_value())
    {
        m_emitterTickRequest->m_emitterIndices.push_back(id);
    }
    else
    {
        ST_EmitterTickRequest request;
        request.m_emitterIndices.push_back(id);
        m_emitterTickRequest = request;
    }
}

void ST_RenderSwapData::UpdateParticleTransform(ST_ParticleEmitterTransformDesc& desc)
{
    if (m_emitterTransformRequest.has_value())
    {
        m_emitterTransformRequest->Add(desc);
    }
    else
    {
        ST_EmitterTransformRequest request;
        request.Add(desc);
        m_emitterTransformRequest = request;
    }
}

NAMESPACE_XYH_END

