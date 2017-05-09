#pragma once

#include "Platform.h"

#include "Window.h"

#include <assert.h>
#include "atlbase.h" 

#if VK_USE_PLATFORM_WIN32_KHR

// Microsoft Windows specific versions of window functions
LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window * window = reinterpret_cast<Window*>(GetWindowLongA(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_SIZE:
		// we get here if the window has changed size, we should rebuild most
		// of our window resources before rendering to this window again.
		// ( no need for this because our window sizing by hand is disabled )
		break;
	default:
		break;
	}
	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

uint64_t	Window::m_Win32ClassIdCounter = 0;

void Window::InitOSWindow()
{
	WNDCLASSEXA win_class{};
	assert(m_Width > 0);
	assert(m_Height > 0);

	m_Win32Instance = GetModuleHandleA(nullptr);
	m_Win32ClassName = m_WindowName + "_" + std::to_string(m_Win32ClassIdCounter);
	m_Win32ClassIdCounter++;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEXA);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = m_Win32Instance; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = m_Win32ClassName.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassExA(&win_class)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// Create window with the registered class:
	RECT wr = { 0, 0, LONG(m_Width), LONG(m_Height) };
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	m_Win32Window = CreateWindowExA(0,
		m_Win32ClassName.c_str(),		// class name
		m_WindowName.c_str(),			// app name
		style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		m_Win32Instance,				// hInstance
		NULL);							// no extra parameters
	if (!m_Win32Window) {
		// It didn't work, so try to give a useful error:
		printf("%d\n", GetLastError());
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongA(m_Win32Window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(m_Win32Window, SW_SHOW);
	SetForegroundWindow(m_Win32Window);
	SetFocus(m_Win32Window);
}

void Window::DestroyOSWindow()
{
	DestroyWindow(m_Win32Window);
	UnregisterClassA(m_Win32ClassName.c_str(), m_Win32Instance);
}

void Window::UpdateOSWindow()
{
	MSG msg;
	if (PeekMessageA(&msg, m_Win32Window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Window::InitOSSurface()
{
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR{};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.hinstance = m_Win32Instance;
	win32SurfaceCreateInfoKHR.hwnd = m_Win32Window;
	vkCreateWin32SurfaceKHR(m_Renderer->GetVulkanInstance(), &win32SurfaceCreateInfoKHR, nullptr, &m_Surface);
}
#endif