#include <Common.h>
#include "RHIResources.h"

NAMESPACE_XYH_BEGIN

struct RHIBufferInitializer
{
	RHIBufferInitializer() = default;
	~RHIBufferInitializer()
	{

	}

protected:
	RHIBuffer* m_buffer = nullptr;
};

NAMESPACE_XYH_END