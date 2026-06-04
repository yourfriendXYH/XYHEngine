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

void CheckLastOpenGLError(const char* prefix, const char* file, long line, const char* operation)
{
	GLenum glerr;
	while ((glerr = glGetError()) != GL_NO_ERROR) {
		switch (glerr) {
		case GL_INVALID_VALUE:
			printf("%s GL_INVALID_VALUE\n", operation);
			break;
		default:
			printf("gl error  0x%x\n", (int)glerr);
			break;
		}
		std::string str = file;
		const int kMaxErrors = 10;
		int counter = 0;
		int pos = str.find_last_of('\\');
		printf("%s:%ld :", str.substr(pos + 1, str.length() - pos).c_str(), line);
		if (prefix) {
			std::string errorString = prefix;
			errorString += ": ";
			const char* gluMsg = reinterpret_cast<const char*>(gluErrorString(glerr));
			if (gluMsg) {
				printf("prefix error  %s\n", gluMsg);

			}
			else {
				printf("prefix error : unkown error 0x%x\n", glerr);
			}
		}
		else {
			const char* gluMsg = reinterpret_cast<const char*>(gluErrorString(glerr));
			if (gluMsg) {
				printf("%s %s\n", operation, gluMsg);
			}
			else {
				printf("%s 0x%x\n", operation, glerr);
			}
		}
		++counter;
		if (counter > kMaxErrors) {
			printf("GL: error count exceeds %i, stop reporting errors\n", kMaxErrors);
			return;
		}
	}
}

NAMESPACE_XYH_END