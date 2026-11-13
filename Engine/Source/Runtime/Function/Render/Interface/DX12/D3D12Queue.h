#pragma once
#include <Common.h>

NAMESPACE_XYH_BEGIN

enum class ED3D12QueueType
{
	Direct = 0,
	Copy,
	Async,

	Count,
};

NAMESPACE_XYH_END