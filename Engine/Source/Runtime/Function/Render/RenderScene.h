#pragma once
#include <optional>
#include <Common.h>
#include "Light.h"
#include "RenderEntity.h"

NAMESPACE_XYH_BEGIN

class RenderScene
{
public:
	RenderScene();
	~RenderScene();

public:
	ST_AmbientLight m_ambientLight;	// 环境光
	ST_DirectionalLight m_directionalLight;	// 直射光
	PointLightList m_pointLightList;	// 点光源列表

	std::vector<RenderEntity> m_renderEntities;	// 渲染实体列表

	std::optional<RenderEntity> m_renderAxis;	// 渲染坐标轴

private:


private:

};

NAMESPACE_XYH_END