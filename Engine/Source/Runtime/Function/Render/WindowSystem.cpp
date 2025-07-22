#include "WindowSystem.h"
#include "../../Core/Macro.h"

NAMESPACE_XYH_BEGIN

WindowSystem::WindowSystem()
{
}

WindowSystem::~WindowSystem()
{
	glfwDestroyWindow(m_pWindow);   // ���ٴ���
	glfwTerminate();    // ���ٴ��ڲ���ֹGLFW
}

void WindowSystem::Initialize(ST_WindowCreateInfo createInfo)
{
	if (!glfwInit())    // ��ʼ��GLFW
    {
        LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
        return;
	}

	m_width = createInfo.m_width;   // ���ô��ڿ��
	m_height = createInfo.m_height; // ���ô��ڸ߶�

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // ���ÿͻ���APIΪ��API
    m_pWindow = glfwCreateWindow(createInfo.m_width, createInfo.m_height, createInfo.m_title, nullptr, nullptr); // ��������
    if (!m_pWindow)
    {
        LOG_FATAL(__FUNCTION__, "failed to create window");
        glfwTerminate();    // ���ٴ��ڲ���ֹGLFW
        return;
	}

    glfwSetWindowUserPointer(m_pWindow, this);   // ���ô����û�ָ��Ϊ��ǰ����

	// ��������ص�
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCharCallback(m_pWindow, CharCallback);
    glfwSetCharModsCallback(m_pWindow, CharModsCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetCursorEnterCallback(m_pWindow, CursorEnterCallback);
    glfwSetScrollCallback(m_pWindow, ScrollCallback);
    glfwSetDropCallback(m_pWindow, DropCallback);
    glfwSetWindowSizeCallback(m_pWindow, WindowSizeCallback);
	glfwSetWindowCloseCallback(m_pWindow, WindowCloseCallback);

	glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE); // ����ԭʼ����˶�ģʽ
}

void WindowSystem::PollEvents()
{
    glfwPollEvents();  // �����¼�
}

bool WindowSystem::ShouldClose() const
{
	return glfwWindowShouldClose(m_pWindow);  // ��鴰���Ƿ�Ӧ�ùر�
}

void WindowSystem::SetTitle(const char* title)
{
    glfwSetWindowTitle(m_pWindow, title);  // ���ô��ڱ���
}

GLFWwindow* WindowSystem::GetWindow() const
{
    return m_pWindow;
}

std::array<int, 2> WindowSystem::GetWindowSize() const
{
    return std::array<int, 2>({ m_width, m_height });
}

bool WindowSystem::IsMouseButtonDown(int button) const
{
    if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        return false;
    }
	return glfwGetMouseButton(m_pWindow, button) == GLFW_PRESS;  // �����갴ť�Ƿ���
}

bool WindowSystem::GetFocusMode() const
{
    return m_isFocusMode;
}

void WindowSystem::SetFocusMode(bool isFocusMode)
{
	m_isFocusMode = isFocusMode;
}

void WindowSystem::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnKey(key, scancode, action, mods);
	}
}

void WindowSystem::CharCallback(GLFWwindow* window, unsigned int codePoint)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnChar(codePoint);
	}
}

void WindowSystem::CharModsCallback(GLFWwindow* window, unsigned int mods, int codePoint)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnCharMods(mods, codePoint);
	}
}

void WindowSystem::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnMouseButton(button, action, mods);
	}
}

void WindowSystem::CursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnCursorPos(xPos, yPos);
	}
}

void WindowSystem::CursorEnterCallback(GLFWwindow* window, int entered)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnCursorEnter(entered);
	}
}

void WindowSystem::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnScroll(xOffset, yOffset);
	}
}

void WindowSystem::DropCallback(GLFWwindow* window, int count, const char** paths)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnDrop(count, paths);
	}
}

void WindowSystem::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(window);
    if (nullptr != pApp)
    {
        pApp->OnWindowSize(width, height);
	}
}

void WindowSystem::WindowCloseCallback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void WindowSystem::OnReset()
{
    for (const auto& func : m_onResetFunc)
    {
        func();
	}
}

void WindowSystem::OnKey(int key, int scancode, int action, int mods)
{
    for (const auto& func : m_onKeyFunc)
    {
        func(key, scancode, action, mods);
	}
}

void WindowSystem::OnChar(unsigned int codePoint)
{
    for (const auto& func : m_onCharFunc)
    {
        func(codePoint);
	}
}

void WindowSystem::OnCharMods(int mods, unsigned int codePoint)
{
    for (const auto& func : m_onCharModsFunc)
    {
        func(mods, codePoint);
	}
}

void WindowSystem::OnMouseButton(int button, int action, int mods)
{
    for (const auto& func : m_onMouseButtonFunc)
    {
        func(button, action, mods);
	}
}

void WindowSystem::OnCursorPos(double xPos, double yPos)
{
    for (const auto& func : m_onCursorPosFunc)
    {
        func(xPos, yPos);
	}
}

void WindowSystem::OnCursorEnter(int entered)
{  
    for (const auto& func : m_onCursorEnterFunc)
    {
        func(entered);
	}
}

void WindowSystem::OnScroll(double xOffset, double yOffset)
{
    for (const auto& func : m_onScrollFunc)
    {
        func(xOffset, yOffset);
	}
}

void WindowSystem::OnDrop(int count, const char** paths)
{
    for (const auto& func : m_onDropFunc)
    {
        func(count, paths);
	}
}

void WindowSystem::OnWindowSize(int width, int height)
{
    for (const auto& func : m_onWindowSizeFunc)
    {
        func(width, height);
	}
}

NAMESPACE_XYH_END

