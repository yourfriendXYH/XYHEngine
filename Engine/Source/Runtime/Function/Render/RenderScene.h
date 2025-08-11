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
	ST_AmbientLight m_ambientLight;	// ������
	ST_DirectionalLight m_directionalLight;	// ֱ���
	PointLightList m_pointLightList;	// ���Դ�б�

	std::vector<RenderEntity> m_renderEntities;	// ��Ⱦʵ���б�

	std::optional<RenderEntity> m_renderAxis;	// ��Ⱦ������

private:


private:

};

NAMESPACE_XYH_END