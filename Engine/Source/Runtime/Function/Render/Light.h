#pragma once
#include <Common.h>
#include "Runtime/Core/Math/Vector3.h"
#include "RenderType.h"

NAMESPACE_XYH_BEGIN

// 点光源
struct ST_PointLight
{
public:
	float CalculateRadius() const
	{
		// radius = where attenuation would lead to an intensity of 1W/m^2
		const float INTENSITY_CUTOFF = 1.0f;	// 光强度截止值
		const float ATTENTUATION_CUTOFF = 0.05f;	// 衰减截止值
		Vector3 intensity = m_flux / (4.0f * Math_PI);	// 计算光强度
		float maxIntensity = Vector3::getMaxElement(intensity);	// 获取最大光强度分量
		float attenuation = Math::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;	// 计算衰减
		return 1.0f / sqrtf(attenuation);
	}

public:
	Vector3 m_position;	// 位置
	Vector3 m_flux;	// 光通量
};

// 环境光
struct ST_AmbientLight
{
	Vector3 m_irradiance;	// 辐照度
};

// 直射光
struct ST_DirectionalLight
{
	Vector3 m_direction;    // 方向
	Vector3 m_color;	// 颜色
};

// 光源列表
struct ST_LightList
{
	// 16字节对齐
	struct ST_PointLightBuffer
	{
		Vector3 m_position;		// 位置
		float m_padding;
		// radiant intensity in W/sr
		// can be calculated from radiant flux
		Vector3 m_intensity;	// 光强
		float m_radius;
	};
};

// 点光源列表
class PointLightList : public ST_LightList
{
public:
	void Init() {}
	void Shutdown() {}
	void Update() {}

public:
	std::vector<ST_PointLight> m_lights;	// 点光源列表
	std::shared_ptr<BufferData> m_buffer;	// 点光源缓冲区数据
};

NAMESPACE_XYH_END