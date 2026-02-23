#include "Editor.h"
#include <cassert>
#include <Runtime/Engine.h>

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
	assert(m_pEngineRuntime);

	float deltaTime;
	while (true)
	{
		// 计算每帧的持续时间
		deltaTime = m_pEngineRuntime->CalculateDeltaTime();

		// 帧执行
		if (!m_pEngineRuntime->TickOneFrame(deltaTime))
		{
			return;	// 窗口关闭时跳出循环
		}
	}
}

NAMESPACE_XYH_END

