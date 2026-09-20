#pragma once
#include <Common.h>
#include "RHIAccess.h"
#include "RHIDefinitions.h"

NAMESPACE_XYH_BEGIN

extern ERHIAccess RHIGetDefaultResourceState(EBufferUsageFlags InUsage, bool bInHasInitialData);

NAMESPACE_XYH_END