#include "Editor.h"
#include <cassert>

NAMESPACE_XYH_BEGIN

XYHEditor::XYHEditor()
{

}

XYHEditor::~XYHEditor()
{
}

void XYHEditor::Initialize(XYHEngine* pEngine)
{
	assert(pEngine);

	m_pEngineRuntime = pEngine;
}

void XYHEditor::Clear()
{

}

void XYHEditor::Run()
{

}

NAMESPACE_XYH_END

