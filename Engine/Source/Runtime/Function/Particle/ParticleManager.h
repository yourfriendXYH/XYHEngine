#pragma once
#include <Common.h>
#include <Runtime/Resource/ResourceType/Global/GlobalParticle.h>

NAMESPACE_XYH_BEGIN

// 粒子管理器
class ParticleManager
{
public:
	ParticleManager() = default;
	~ParticleManager() {};

	const GlobalParticleResource& GetGlobalParticleRes() const;

private:
	GlobalParticleResource m_globalParticleRes;	// 全局粒子资源
};

NAMESPACE_XYH_END