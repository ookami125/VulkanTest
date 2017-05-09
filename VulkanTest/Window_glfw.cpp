#include "BUILD_OPTIONS.h"
#include "Platform.h"

#include "Window.h"
#include "Shared.h"
#include "Renderer.h"

#include <assert.h>
#include <iostream>

#if USE_FRAMEWORK_GLFW

void Window::InitOSWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_GlfwWindow = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
	glfwGetFramebufferSize(m_GlfwWindow, (int*)&m_Width, (int*)&m_Height);
}

void Window::DestroyOSWindow()
{
	glfwDestroyWindow(m_GlfwWindow);
}

void Window::UpdateOSWindow()
{
	glfwPollEvents();

	if (glfwWindowShouldClose(m_GlfwWindow)) {
		Close();
	}
}

void Window::InitOSSurface()
{
	VkInstance instance = m_Renderer->GetVulkanInstance();
	ErrorCheck(glfwCreateWindowSurface(instance, m_GlfwWindow, nullptr, &m_Surface));
}

#endif