#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Common.h>
#include <array>
#include <functional>

NAMESPACE_XYH_BEGIN

struct ST_WindowCreateInfo
{
	int m_width = 1280;	// 窗口宽度
	int m_height = 720;	// 窗口高度
	const char* m_title = "XYH Engine";	// 窗口标题
	bool m_fullscreen = false;	// 是否全屏
};

// 窗口系统
class WindowSystem
{
public:
	WindowSystem();
	virtual ~WindowSystem();

	void Initialize(ST_WindowCreateInfo createInfo);	// 初始化窗口

	void PollEvents();	// 处理事件

	bool ShouldClose() const;	// 窗口是否应该关闭

	void SetTitle(const char* title);	// 设置窗口标题

	GLFWwindow* GetWindow() const;	// 获取窗口指针

	std::array<int, 2> GetWindowSize() const;	// 获取窗口大小

	using OnResetFunc = std::function<void()>;	// 窗口重置回调
	using OnKeyFunc = std::function<void(int, int, int, int)>;	// 键盘事件回调
	using OnCharFunc = std::function<void(unsigned int)>;	// 字符输入回调
	using OnCharModsFunc = std::function<void(int, unsigned int)>;	// 字符输入和修饰键回调
	using OnMouseButtonFunc = std::function<void(int, int, int)>;	// 鼠标按钮事件回调
	using OnCursorPosFunc = std::function<void(double, double)>;	// 鼠标位置事件回调
	using OnCursorEnterFunc = std::function<void(int)>;	// 鼠标进入窗口事件回调
	using OnScrollFunc = std::function<void(double, double)>;	// 鼠标滚轮事件回调
	using OnDropFunc = std::function<void(int, const char**)>;	// 拖放文件事件回调
	using OnWindowSizeFunc = std::function<void(int, int)>;	// 窗口大小变化事件回调
	using OnWindowCloseFunc = std::function<void()>;	// 窗口关闭事件回调

	// 注册回调函数
	void RegisterOnResetFunc(OnResetFunc func) { m_onResetFunc.push_back(func); }	// 注册窗口重置回调
	void RegisterOnKeyFunc(OnKeyFunc func) { m_onKeyFunc.push_back(func); };	// 注册键盘事件回调
	void RegisterOnCharFunc(OnCharFunc func) { m_onCharFunc.push_back(func); }	// 注册字符输入回调
	void RegisterOnCharModsFunc(OnCharModsFunc func) { m_onCharModsFunc.push_back(func); }	// 注册字符输入和修饰键回调
	void RegisterOnMouseButtonFunc(OnMouseButtonFunc func) { m_onMouseButtonFunc.push_back(func); }	// 注册鼠标按钮事件回调
	void RegisterOnCursorPosFunc(OnCursorPosFunc func) { m_onCursorPosFunc.push_back(func); }	// 注册鼠标位置事件回调
	void RegisterOnCursorEnterFunc(OnCursorEnterFunc func) { m_onCursorEnterFunc.push_back(func); }	// 注册鼠标进入窗口事件回调
	void RegisterOnScrollFunc(OnScrollFunc func) { m_onScrollFunc.push_back(func); }	// 注册鼠标滚轮事件回调
	void RegisterOnDropFunc(OnDropFunc func) { m_onDropFunc.push_back(func); }	// 注册拖放文件事件回调
	void RegisterOnWindowSizeFunc(OnWindowSizeFunc func) { m_onWindowSizeFunc.push_back(func); }	// 注册窗口大小变化事件回调
	void RegisterOnWindowCloseFunc(OnWindowCloseFunc func) { m_onWindowCloseFunc.push_back(func); }	// 注册窗口关闭事件回调

	bool IsMouseButtonDown(int button) const;	// 检查鼠标按钮是否按下
	bool GetFocusMode() const;	// 获取焦点模式状态
	void SetFocusMode(bool isFocusMode);	// 设置焦点模式

protected:
	// 窗口事件回调函数
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);	// 键盘事件回调
	static void CharCallback(GLFWwindow* window, unsigned int codePoint);	// 字符输入回调
	static void CharModsCallback(GLFWwindow* window, unsigned int mods, int codePoint);	// 字符输入和修饰键回调
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);	// 鼠标按钮事件回调
	static void CursorPosCallback(GLFWwindow* window, double xPos, double yPos);	// 鼠标位置事件回调
	static void CursorEnterCallback(GLFWwindow* window, int entered);	// 鼠标进入窗口事件回调
	static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);	// 鼠标滚轮事件回调
	static void DropCallback(GLFWwindow* window, int count, const char** paths);	// 拖放文件事件回调
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);	// 窗口大小变化事件回调
	static void WindowCloseCallback(GLFWwindow* window);	// 窗口关闭事件回调

	void OnReset();	// 窗口重置事件处理
	void OnKey(int key, int scancode, int action, int mods);	// 键盘事件处理
	void OnChar(unsigned int codePoint);	// 字符输入事件处理
	void OnCharMods(int mods, unsigned int codePoint);	// 字符输入和修饰键事件处理
	void OnMouseButton(int button, int action, int mods);	// 鼠标按钮事件处理
	void OnCursorPos(double xPos, double yPos);	// 鼠标位置事件处理
	void OnCursorEnter(int entered);	// 鼠标进入窗口事件处理
	void OnScroll(double xOffset, double yOffset);	// 鼠标滚轮事件处理
	void OnDrop(int count, const char** paths);	// 拖放文件事件处理
	void OnWindowSize(int width, int height);	// 窗口大小变化事件处理

private:
	GLFWwindow* m_pWindow = nullptr;	// 窗口指针
	int m_width = 0;	// 窗口宽度
	int m_height = 0;	// 窗口高度

	bool m_isFocusMode = false;	// 是否为焦点模式

	std::vector<OnResetFunc>       m_onResetFunc;
	std::vector<OnKeyFunc>         m_onKeyFunc;
	std::vector<OnCharFunc>        m_onCharFunc;
	std::vector<OnCharModsFunc>    m_onCharModsFunc;
	std::vector<OnMouseButtonFunc> m_onMouseButtonFunc;
	std::vector<OnCursorPosFunc>   m_onCursorPosFunc;
	std::vector<OnCursorEnterFunc> m_onCursorEnterFunc;
	std::vector<OnScrollFunc>      m_onScrollFunc;
	std::vector<OnDropFunc>        m_onDropFunc;
	std::vector<OnWindowSizeFunc>  m_onWindowSizeFunc;
	std::vector<OnWindowCloseFunc> m_onWindowCloseFunc;

};

NAMESPACE_XYH_END