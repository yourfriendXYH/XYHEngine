#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Common.h>
#include <array>
#include <functional>

NAMESPACE_XYH_BEGIN

struct ST_WindowCreateInfo
{
	int m_width = 1280;	// ���ڿ��
	int m_height = 720;	// ���ڸ߶�
	const char* m_title = "XYH Engine";	// ���ڱ���
	bool m_fullscreen = false;	// �Ƿ�ȫ��
};

// ����ϵͳ
class WindowSystem
{
public:
	WindowSystem();
	virtual ~WindowSystem();

	void Initialize(ST_WindowCreateInfo createInfo);	// ��ʼ������

	void PollEvents();	// �����¼�

	bool ShouldClose() const;	// �����Ƿ�Ӧ�ùر�

	void SetTitle(const char* title);	// ���ô��ڱ���

	GLFWwindow* GetWindow() const;	// ��ȡ����ָ��

	std::array<int, 2> GetWindowSize() const;	// ��ȡ���ڴ�С

	using OnResetFunc = std::function<void()>;	// �������ûص�
	using OnKeyFunc = std::function<void(int, int, int, int)>;	// �����¼��ص�
	using OnCharFunc = std::function<void(unsigned int)>;	// �ַ�����ص�
	using OnCharModsFunc = std::function<void(int, unsigned int)>;	// �ַ���������μ��ص�
	using OnMouseButtonFunc = std::function<void(int, int, int)>;	// ��갴ť�¼��ص�
	using OnCursorPosFunc = std::function<void(double, double)>;	// ���λ���¼��ص�
	using OnCursorEnterFunc = std::function<void(int)>;	// �����봰���¼��ص�
	using OnScrollFunc = std::function<void(double, double)>;	// �������¼��ص�
	using OnDropFunc = std::function<void(int, const char**)>;	// �Ϸ��ļ��¼��ص�
	using OnWindowSizeFunc = std::function<void(int, int)>;	// ���ڴ�С�仯�¼��ص�
	using OnWindowCloseFunc = std::function<void()>;	// ���ڹر��¼��ص�

	// ע��ص�����
	void RegisterOnResetFunc(OnResetFunc func) { m_onResetFunc.push_back(func); }	// ע�ᴰ�����ûص�
	void RegisterOnKeyFunc(OnKeyFunc func) { m_onKeyFunc.push_back(func); };	// ע������¼��ص�
	void RegisterOnCharFunc(OnCharFunc func) { m_onCharFunc.push_back(func); }	// ע���ַ�����ص�
	void RegisterOnCharModsFunc(OnCharModsFunc func) { m_onCharModsFunc.push_back(func); }	// ע���ַ���������μ��ص�
	void RegisterOnMouseButtonFunc(OnMouseButtonFunc func) { m_onMouseButtonFunc.push_back(func); }	// ע����갴ť�¼��ص�
	void RegisterOnCursorPosFunc(OnCursorPosFunc func) { m_onCursorPosFunc.push_back(func); }	// ע�����λ���¼��ص�
	void RegisterOnCursorEnterFunc(OnCursorEnterFunc func) { m_onCursorEnterFunc.push_back(func); }	// ע�������봰���¼��ص�
	void RegisterOnScrollFunc(OnScrollFunc func) { m_onScrollFunc.push_back(func); }	// ע���������¼��ص�
	void RegisterOnDropFunc(OnDropFunc func) { m_onDropFunc.push_back(func); }	// ע���Ϸ��ļ��¼��ص�
	void RegisterOnWindowSizeFunc(OnWindowSizeFunc func) { m_onWindowSizeFunc.push_back(func); }	// ע�ᴰ�ڴ�С�仯�¼��ص�
	void RegisterOnWindowCloseFunc(OnWindowCloseFunc func) { m_onWindowCloseFunc.push_back(func); }	// ע�ᴰ�ڹر��¼��ص�

	bool IsMouseButtonDown(int button) const;	// �����갴ť�Ƿ���
	bool GetFocusMode() const;	// ��ȡ����ģʽ״̬
	void SetFocusMode(bool isFocusMode);	// ���ý���ģʽ

protected:
	// �����¼��ص�����
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);	// �����¼��ص�
	static void CharCallback(GLFWwindow* window, unsigned int codePoint);	// �ַ�����ص�
	static void CharModsCallback(GLFWwindow* window, unsigned int mods, int codePoint);	// �ַ���������μ��ص�
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);	// ��갴ť�¼��ص�
	static void CursorPosCallback(GLFWwindow* window, double xPos, double yPos);	// ���λ���¼��ص�
	static void CursorEnterCallback(GLFWwindow* window, int entered);	// �����봰���¼��ص�
	static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);	// �������¼��ص�
	static void DropCallback(GLFWwindow* window, int count, const char** paths);	// �Ϸ��ļ��¼��ص�
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);	// ���ڴ�С�仯�¼��ص�
	static void WindowCloseCallback(GLFWwindow* window);	// ���ڹر��¼��ص�

	void OnReset();	// ���������¼�����
	void OnKey(int key, int scancode, int action, int mods);	// �����¼�����
	void OnChar(unsigned int codePoint);	// �ַ������¼�����
	void OnCharMods(int mods, unsigned int codePoint);	// �ַ���������μ��¼�����
	void OnMouseButton(int button, int action, int mods);	// ��갴ť�¼�����
	void OnCursorPos(double xPos, double yPos);	// ���λ���¼�����
	void OnCursorEnter(int entered);	// �����봰���¼�����
	void OnScroll(double xOffset, double yOffset);	// �������¼�����
	void OnDrop(int count, const char** paths);	// �Ϸ��ļ��¼�����
	void OnWindowSize(int width, int height);	// ���ڴ�С�仯�¼�����

private:
	GLFWwindow* m_pWindow = nullptr;	// ����ָ��
	int m_width = 0;	// ���ڿ��
	int m_height = 0;	// ���ڸ߶�

	bool m_isFocusMode = false;	// �Ƿ�Ϊ����ģʽ

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