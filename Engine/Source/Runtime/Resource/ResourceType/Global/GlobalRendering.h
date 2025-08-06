#pragma once
#include <Common.h>
#include <string>

NAMESPACE_XYH_BEGIN

class SkyBoxIrradianceMap
{
public:
    std::string m_negativeXMap;
    std::string m_positiveXMap;
    std::string m_negativeYMap;
    std::string m_positiveYMap;
    std::string m_negativeZMap;
    std::string m_positiveZMap;
};

class SkyBoxSpecularMap
{
public:
    std::string m_negativeXMap;
    std::string m_positiveXMap;
    std::string m_negativeYMap;
    std::string m_positiveYMap;
    std::string m_negativeZMap;
    std::string m_positiveZMap;
};

class GlobalRenderingResource
{
public:
	GlobalRenderingResource();
	~GlobalRenderingResource();

private:

};

NAMESPACE_XYH_END