#pragma once
#include <Runtime/Function/Render/Interface/RHI.h>

NAMESPACE_XYH_BEGIN

class D3D12RHI final : public RHI
{
public:

	virtual void Initialize(ST_RHIInitInfo initInfo) override final;

	virtual ~D3D12RHI() override final;

private:

	void CreateDevice();

};

NAMESPACE_XYH_END