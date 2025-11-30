#pragma once
#include <Common.h>
#include <Runtime/Core/Math/Vector4.h>
#include <Runtime/Core/Math/Matrix4.h>
#include <Runtime/Core/Math/Vector2.h>
#include <Runtime/Core/Math/Vector3.h>
#include "EmitterIdAllocator.h"

NAMESPACE_XYH_BEGIN

struct ST_ParticleEmitterTransformDesc
{
	ParticleEmitterID m_id;
	Vector4 m_position;
	Matrix4x4 m_rotation;
};

struct ST_ParticleEmitterDesc
{
	Vector4 m_position;
	Matrix4x4 m_rotation;
	Vector4 m_velocity;
	Vector4 m_acceleration;
	Vector3 m_size;
	int m_emitterType;
	Vector2 m_life;
	Vector2 m_padding;
	Vector4 m_color;

	ST_ParticleEmitterDesc() = default;

	//ST_ParticleEmitterDesc(const ParticleComponentRes& componentRes, ST_ParticleEmitterTransformDesc& transformDesc) :
	//	m_position(transformDesc.m_position),
	//	m_rotation(transformDesc.m_rotation),
	//	m_velocity(componentRes.m_velocity),
	//	m_acceleration(componentRes.m_acceleration),
	//	m_size(componentRes.m_size),
	//	m_emitterType(componentRes.m_emitter_type),
	//	m_life(componentRes.m_life),
	//	m_color(componentRes.m_color)
	//{
	//}
};

NAMESPACE_XYH_END