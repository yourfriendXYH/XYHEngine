#include "OpenGLRHI.h"
#include <Runtime/Function/Render/WindowSystem.h>

NAMESPACE_XYH_BEGIN

void OpenGLRHI::Initialize(ST_RHIInitInfo initInfo)
{
	m_pGLFWwindow = initInfo.m_pWindowSystem->GetWindow();	// 获取窗口
	std::array<int, 2> windowSize = initInfo.m_pWindowSystem->GetWindowSize(); // 获取窗口大小
	m_viewport = { 0.0f, 0.0f, static_cast<float>(windowSize[0]), static_cast<float>(windowSize[1]), 0.0f, 1.0f }; // 设置视口大小
	m_scissor = { { 0, 0 }, { (uint32_t)windowSize[0], (uint32_t)windowSize[1] } }; // 设置裁剪区域大小

	glfwMakeContextCurrent(m_pGLFWwindow);

	if (glewInit() != GLEW_OK)
		return;
}

OpenGLRHI::~OpenGLRHI()
{
}

void OpenGLRHI::CreateSwapChain()
{
}

void OpenGLRHI::WaitForFences()
{
}

void OpenGLRHI::ResetCommandPool()
{
}

uint8_t OpenGLRHI::GetCurrentFrameIndex() const
{
	return 0;
}

void OpenGLRHI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
{
	// 交换前后缓冲区
	glfwSwapBuffers(m_pGLFWwindow);
}

NAMESPACE_XYH_END