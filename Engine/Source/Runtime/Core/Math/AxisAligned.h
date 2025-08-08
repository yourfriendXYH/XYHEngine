#pragma once
#include <Common.h>
#include "Vector3.h"
#include <limits>

NAMESPACE_XYH_BEGIN

class AxisAlignedBox
{
public:
    AxisAlignedBox() {}
    AxisAlignedBox(const Vector3 & center, const Vector3 & half_extent);

    void Merge(const Vector3 & new_point);
    void Update(const Vector3 & center, const Vector3 & half_extent);

    const Vector3& GetCenter() const { return m_center; }
    const Vector3& GetHalfExtent() const { return m_half_extent; }
    const Vector3& GetMinCorner() const { return m_min_corner; }
    const Vector3& GetMaxCorner() const { return m_max_corner; }

private:
    Vector3 m_center{ Vector3::ZERO };
    Vector3 m_half_extent{ Vector3::ZERO };

    Vector3 m_min_corner{
        std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    Vector3 m_max_corner{
        -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };
};

NAMESPACE_XYH_END