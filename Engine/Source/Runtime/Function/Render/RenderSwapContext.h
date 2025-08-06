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


class RenderSwapContext
{
public:

private:

};

NAMESPACE_XYH_END