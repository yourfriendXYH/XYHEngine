#pragma once
#include "../Common/Common.h"

NAMESPACE_XYH_BEGIN

// class EditorUI;
class XYHEngine;

class XYHEditor
{
public:
	XYHEditor();
	virtual ~XYHEditor();

	void Initialize(XYHEngine* pEngine);

	void Clear();

	void Run();

private:
	XYHEngine* m_pEngineRuntime = nullptr;
};

NAMESPACE_XYH_END
