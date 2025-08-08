#include "AxisAligned.h"

NAMESPACE_XYH_BEGIN

AxisAlignedBox::AxisAlignedBox(const Vector3& center, const Vector3& half_extent) 
{ 
    Update(center, half_extent); 
}

void AxisAlignedBox::Merge(const Vector3& new_point)
{
    m_min_corner.makeFloor(new_point);
    m_max_corner.makeCeil(new_point);

    m_center = 0.5f * (m_min_corner + m_max_corner);
    m_half_extent = m_center - m_min_corner;
}

void AxisAlignedBox::Update(const Vector3& center, const Vector3& half_extent)
{
    m_center = center;
    m_half_extent = half_extent;
    m_min_corner = center - half_extent;
    m_max_corner = center + half_extent;
}

NAMESPACE_XYH_END