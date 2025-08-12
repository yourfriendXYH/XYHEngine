#pragma once
#include <Common.h>
#include <atomic>
#include <limits>

NAMESPACE_XYH_BEGIN

using GObjectID = std::size_t;

constexpr GObjectID k_invalidGObjectId = std::numeric_limits<std::size_t>::max();

NAMESPACE_XYH_END