#pragma once
#include <Common.h>
#include <Runtime/Core/Math/Matrix4.h>
#include "Runtime/Function/Framework/Object/ObjectIdAllocator.h"

NAMESPACE_XYH_BEGIN

constexpr size_t k_invalidPartId = std::numeric_limits<size_t>::max();

struct ST_GameObjectMeshDesc
{
    std::string m_meshFile;
};

struct ST_GameObjectMaterialDesc
{
    std::string m_baseColorTextureFile;
    std::string m_metallicRoughnessTextureFile;
    std::string m_normalTextureFile;
    std::string m_occlusionTextureFile;
    std::string m_emissiveTextureFile;
    bool m_withTexture{ false };
};

struct ST_GameObjectTransformDesc
{
    Matrix4x4 m_transformMatrix{ Matrix4x4::IDENTITY };
};

struct ST_SkeletonAnimationResultTransform
{
    Matrix4x4 m_matrix;
};

struct ST_SkeletonAnimationResult
{
    std::vector<ST_SkeletonAnimationResultTransform> m_transforms;
};

// 需要反射
struct ST_GameObjectPartDesc
{
    ST_GameObjectMeshDesc m_meshDesc;   // 网格资源

    ST_GameObjectMaterialDesc m_materialDesc;   // 材质资源

    ST_GameObjectTransformDesc m_transformDesc; // 变换矩阵

    bool m_withAnimation{ false };  // 是否有动画

    ST_SkeletonAnimationResult m_skeletonAnimationResult;   // 骨骼动画结果
};

struct ST_GameObjectPartId
{
    GObjectID m_goId = k_invalidGObjectId;
    size_t m_partId = k_invalidPartId;

    bool   operator==(const ST_GameObjectPartId& rhs) const { return m_goId == rhs.m_goId && m_partId == rhs.m_partId; }
    size_t getHashValue() const { return m_goId ^ (m_partId << 1); }
    bool   isValid() const { return m_goId != k_invalidGObjectId && m_partId != k_invalidPartId; }
};

class GameObjectDesc
{
public:
    GameObjectDesc() : m_goId(0) {}
    GameObjectDesc(size_t go_id, const std::vector<ST_GameObjectPartDesc>& parts) 
        :
        m_goId(go_id), m_objectParts(parts)
    {
    }

    GObjectID GetId() const { return m_goId; }
    const std::vector<ST_GameObjectPartDesc>& GetObjectParts() const { return m_objectParts; }

private:
    GObjectID m_goId{ k_invalidGObjectId };
    std::vector<ST_GameObjectPartDesc> m_objectParts;
};

NAMESPACE_XYH_END

template<>
struct std::hash<XYH::ST_GameObjectPartId>
{
    size_t operator()(const XYH::ST_GameObjectPartId& rhs) const noexcept { return rhs.getHashValue(); }
};