#pragma once
#include <Common.h>
#include "Runtime/Core/Math/Vector3.h"
#include "RenderType.h"

NAMESPACE_XYH_BEGIN

// ���Դ
struct ST_PointLight
{
public:
	float CalculateRadius() const
	{
		// radius = where attenuation would lead to an intensity of 1W/m^2
		const float INTENSITY_CUTOFF = 1.0f;	// ��ǿ�Ƚ�ֵֹ
		const float ATTENTUATION_CUTOFF = 0.05f;	// ˥����ֵֹ
		Vector3 intensity = m_flux / (4.0f * Math_PI);	// �����ǿ��
		float maxIntensity = Vector3::getMaxElement(intensity);	// ��ȡ����ǿ�ȷ���
		float attenuation = Math::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;	// ����˥��
		return 1.0f / sqrtf(attenuation);
	}

public:
	Vector3 m_position;	// λ��
	Vector3 m_flux;	// ��ͨ��
};

// ������
struct ST_AmbientLight
{
	Vector3 m_irradiance;	// ���ն�
};

// ֱ���
struct ST_DirectionalLight
{
	Vector3 m_direction;    // ����
	Vector3 m_color;	// ��ɫ
};

// ��Դ�б�
struct ST_LightList
{
	// 16�ֽڶ���
	struct ST_PointLightBuffer
	{
		Vector3 m_position;		// λ��
		float m_padding;
		// radiant intensity in W/sr
		// can be calculated from radiant flux
		Vector3 m_intensity;	// ��ǿ
		float m_radius;
	};
};

// ���Դ�б�
class PointLightList : public ST_LightList
{
public:
	void Init() {}
	void Shutdown() {}
	void Update() {}

public:
	std::vector<ST_PointLight> m_lights;	// ���Դ�б�
	std::shared_ptr<BufferData> m_buffer;	// ���Դ����������
};

NAMESPACE_XYH_END