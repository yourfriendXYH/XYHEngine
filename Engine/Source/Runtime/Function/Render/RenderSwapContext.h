#pragma once
#include <Common.h>
#include <string>
#include "../../Resource/ResourceType/Global/GlobalRendering.h"

NAMESPACE_XYH_BEGIN

/// �ؿ�IBL��Դ����
struct ST_LevelIBLResourceDesc
{
	SkyBoxIrradianceMap m_skyboxIrradianceMap;  // ��պз��ն���ͼ
	SkyBoxSpecularMap m_skyboxSpecularMap;  // ��պи߹���ͼ
	std::string m_brdfMap;	// BRDF��ͼ
};

/// �ؿ���ɫ�ּ���Դ����
struct ST_LevelColorGradingResourceDesc
{
	std::string m_colorGradingMap;	// ��ɫ�ּ���ͼ
};

// �ؿ���Դ����
struct ST_LevelResourceDesc
{
	ST_LevelIBLResourceDesc m_iblResourceDesc;	// IBL��Դ����
	ST_LevelColorGradingResourceDesc m_colorGradingResourceDesc;	// ��ɫ�ּ���Դ����
};


class RenderSwapContext
{
public:

private:

};

NAMESPACE_XYH_END