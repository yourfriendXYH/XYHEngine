#pragma once
#include <Common.h>
#include "Runtime/Function/Framework/Object/ObjectIdAllocator.h"

NAMESPACE_XYH_BEGIN

constexpr size_t k_invalidPartId = std::numeric_limits<size_t>::max();

struct ST_GameObjectPartId
{
    GObjectID m_goId = k_invalidGObjectId;
    size_t m_partId = k_invalidPartId;

    bool   operator==(const ST_GameObjectPartId& rhs) const { return m_goId == rhs.m_goId && m_partId == rhs.m_partId; }
    size_t getHashValue() const { return m_goId ^ (m_partId << 1); }
    bool   isValid() const { return m_goId != k_invalidGObjectId && m_partId != k_invalidPartId; }
};

NAMESPACE_XYH_END

template<>
struct std::hash<XYH::ST_GameObjectPartId>
{
    size_t operator()(const XYH::ST_GameObjectPartId& rhs) const noexcept { return rhs.getHashValue(); }
};