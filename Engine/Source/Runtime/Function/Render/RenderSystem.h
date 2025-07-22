#pragma once
#include <memory>
#include <Common.h>

NAMESPACE_XYH_BEGIN

class RHI;

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

	std::shared_ptr<RHI> m_pRHI;

};

NAMESPACE_XYH_END