#pragma once
#include <Common.h>
#include <Runtime/Core/Math/Vector3.h>

NAMESPACE_XYH_BEGIN

// TODO 需要支持反射
class GlobalParticleResource
{
public:
	GlobalParticleResource() {}

public:
    int m_emitGap;
    int m_emitCount;
    float m_timeStep;
    float m_maxLife;
    Vector3 m_gravity;
    std::string m_particleBillboardTexturePath;
    std::string m_piccoloLogoTexturePath;
};

NAMESPACE_XYH_END