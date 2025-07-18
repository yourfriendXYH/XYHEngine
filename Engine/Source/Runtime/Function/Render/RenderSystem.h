#pragma once
#include <memory>
#include <Common.h>

NAMESPACE_XYH_BEGIN

struct ST_RenderSystemInitInfo
{

};

class RenderSystem
{
public:
	RenderSystem() = default;
	~RenderSystem();

	void Initialize(ST_RenderSystemInitInfo initInfo);
	void Tick(float deltaTime);
	void Clear();


private:

};

NAMESPACE_XYH_END