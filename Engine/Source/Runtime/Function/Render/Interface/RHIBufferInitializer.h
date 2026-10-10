#pragma once
#include <Common.h>
#include "RHIResources.h"

NAMESPACE_XYH_BEGIN

struct ST_RHIBufferInitializer
{
	ST_RHIBufferInitializer() = default;
	~ST_RHIBufferInitializer()
	{
		buffer = nullptr;
	}

//protected:
	RHIBuffer* buffer = nullptr;
};

NAMESPACE_XYH_END