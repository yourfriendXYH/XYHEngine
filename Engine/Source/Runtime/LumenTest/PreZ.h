#pragma once
#include <Common.h>
#include <Runtime/Function/Render/Interface/RHI.h>

NAMESPACE_XYH_BEGIN

void InitPreZPass(
	std::shared_ptr<RHI> inRHI,
	RHIBuffer* inPositionBuffer, 
	RHIBuffer* inAttributeBuffer,
	RHIBuffer* inIndexBuffer,
	RHITexture* inDSRT
);

void ExecutePreZPass(std::shared_ptr<RHI> inRHI);

NAMESPACE_XYH_END