#include "utils.h"
#include "oglcontext.h"
#include "scene.h"
#if _DEBUG
#pragma comment( linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#else
#endif

HGLRC CreateNBRC(HDC dc)
{
	HGLRC rc = nullptr;
	GLint attribs[]{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_RED_BITS_ARB,8,
		WGL_GREEN_BITS_ARB,8,
		WGL_BLUE_BITS_ARB,8,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,24,
		WGL_STENCIL_BITS_ARB,8,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB,1,
		NULL,NULL
	};
	int pixelFormat[256] = { 0 };
	UINT formatNum = 0;
	wglChoosePixelFormatARB(dc, attribs, NULL, 256, pixelFormat, &formatNum);
	printf("support format nb format num : [%u]\n", formatNum);
	if (formatNum > 0)
	{
		PIXELFORMATDESCRIPTOR pfd;
		DescribePixelFormat(dc, pixelFormat[0], sizeof(pfd), &pfd);
		SetPixelFormat(dc, pixelFormat[0], &pfd);

		int contexAttributes[] = {
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0 
		};
		rc = wglCreateContextAttribsARB(dc, nullptr, contexAttributes);
	}
	return rc;
}
LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = NULL;
	wndclass.hIconSm = NULL;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = GLWindowProc;
	wndclass.lpszClassName = L"GLWindow";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW;
	ATOM atom = RegisterClassEx(&wndclass);
	if (!atom) {
		MessageBox(NULL, L"Register Fail", L"Error", MB_OK);
		return 0;
	}
	int prefered_canvas_width = 1280;
	int prefered_canvas_height = 720;
	RECT rect;
	rect.left = 0;
	rect.right = prefered_canvas_width;
	rect.top = 0;
	rect.bottom = prefered_canvas_height;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);
	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;
	HWND hwnd = CreateWindowEx(NULL, L"GLWindow", L"OpenGL Window", WS_OVERLAPPEDWINDOW,
		100, 100, windowWidth, windowHeight,
		NULL, NULL, hInstance, NULL);
	HDC dc = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	int pixelFormat = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pixelFormat, &pfd);
	HGLRC rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);
	glewInit();
	if (wglChoosePixelFormatARB)
	{
		//destroy window
		wglMakeCurrent(dc, nullptr);
		wglDeleteContext(rc);
		rc = nullptr;
		ReleaseDC(hwnd, dc);
		dc = nullptr;
		DestroyWindow(hwnd);
		hwnd = CreateWindowEx(NULL, L"GLWindow", L"RenderWindow", WS_OVERLAPPEDWINDOW, 100, 100, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
		//create msaa rc
		dc = GetDC(hwnd);
		rc = CreateNBRC(dc);
		wglMakeCurrent(dc, rc);
		const char* glVersionStr = (char*)glGetString(GL_VERSION);
		printf("OpenGL Version : %s\n", glVersionStr);
		Alice::InitExtensions();
	}
	const char* extensionName = Alice::LikelyToSupportExtension("shader_ballot");
	if (extensionName != nullptr) {
		printf("support [%s]\n", extensionName);
	}
	extensionName = Alice::LikelyToSupportExtension("group_vote");
	if (extensionName != nullptr) {
		printf("support [%s]\n", extensionName);
	}
	extensionName = Alice::LikelyToSupportExtension("shader_int64");
	if (extensionName != nullptr) {
		printf("support [%s]\n", extensionName);
	}
	extensionName = Alice::LikelyToSupportExtension("atomic_int64");
	if (extensionName != nullptr) {
		printf("support [%s]\n", extensionName);
	}
	extensionName = Alice::LikelyToSupportExtension("shader5");
	if (extensionName != nullptr) {
		printf("support [%s]\n", extensionName);
	}
	Init(prefered_canvas_width,prefered_canvas_height);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	MSG msg;
	DWORD last_time = timeGetTime();
	while (true)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			DWORD current_time = timeGetTime();//ms
			DWORD frameTime = current_time - last_time;
			last_time = current_time;
			float frameTimeInSecond = float(frameTime) / 1000.0f;
			RenderOneFrame(frameTimeInSecond);
			SwapBuffers(dc);
		}
	}
	return 0;
}