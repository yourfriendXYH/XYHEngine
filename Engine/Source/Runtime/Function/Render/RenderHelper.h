#pragma once
#include <Common.h>
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

NAMESPACE_XYH_BEGIN

// œÚ…œ∂‘∆Î
static inline uint32_t RoundUp(uint32_t value, uint32_t alignment)
{
	uint32_t temp = value + alignment - static_cast<uint32_t>(1);
	return (temp - temp % alignment);
}

NAMESPACE_XYH_END