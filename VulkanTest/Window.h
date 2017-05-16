#pragma once
#include "Platform.h"
#include "Scene.h"
#include "Pipeline.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <array>

class Renderer;

class Window
{
private:
	Renderer* m_Renderer = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	
	uint32_t m_Width = 512;
	uint32_t m_Height = 512;
	std::string m_WindowName;
	uint32_t m_SwapchainImageCount = 2;
	uint32_t m_ActiveSwapchainImageId = UINT32_MAX;

	VkFence m_SwapchainImageAvailable = VK_NULL_HANDLE;

	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
	std::vector<VkFramebuffer> m_Framebuffers; 
	std::vector<Pipeline*> m_Pipelines;
	std::vector<VkCommandBuffer> m_RenderCommandBuffers;
	std::vector<VkSemaphore> m_RenderComplete;

	VkImage	m_DepthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory m_DepthStencilMemory= VK_NULL_HANDLE;
	VkImageView m_DepthStencilImageView = VK_NULL_HANDLE;

	VkSurfaceFormatKHR m_SurfaceFormat{};
	VkSurfaceCapabilitiesKHR m_SurfaceCapabilities{};
	
	VkFormat m_DepthStencilFormat = VK_FORMAT_UNDEFINED;
	bool m_DepthStencilAvailable = false;
	bool m_ShouldClose = false;

public:
#if USE_FRAMEWORK_GLFW
	GLFWwindow* m_GlfwWindow = nullptr;
#elif VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE m_Win32Instance = NULL;
	HWND m_Win32Window = NULL;
	std::string m_Win32ClassName;
	static uint64_t m_Win32ClassIdCounter;
#elif VK_USE_PLATFORM_XCB_KHR
	xcb_connection_t* _xcb_connection = nullptr;
	xcb_screen_t* _xcb_screen = nullptr;
	xcb_window_t _xcb_window = 0;
	xcb_intern_atom_reply_t* _xcb_atom_window_reply = nullptr;
#endif

private:
	void InitOSWindow();
	void DestroyOSWindow();
	void UpdateOSWindow();
	void InitOSSurface();

	void InitSurface();
	void DestroySurface();

	void InitSwapchain();
	void DestroySwapchain();

	void InitSwapchainImages();
	void DestroySwapchainImages();

	void InitDepthStencilImage();
	void DestroyDepthStencilImage();

	void InitRenderPass();
	void DestroyRenderPass();

	void InitFramebuffers();
	void DestroyFramebuffers();

	void InitSyncronizations();
	void DestroySyncronizations();

	void CreatePipelines();
	void DestroyPipelines();

public:
	Window(Renderer* _renderer, uint32_t width, uint32_t height, char* name);
	~Window();

	void Close();
	bool Update();

	void BeginRender();
	void EndRender( std::vector<VkSemaphore> wait_semaphores );

	void RenderScene(const Scene * scene, bool force_recalculate);
	const std::vector<Pipeline*>& GetPipelines();

	VkRenderPass GetVulkanRenderPass();
	VkFramebuffer GetVulkanActiveFramebuffer();
	VkExtent2D GetVulkanSurfaceSize();
	uint32_t GetCurrentFrameBufferIndex();
	std::vector<VkFramebuffer> GetVulkanFramebuffers();
	void Render(const std::vector<VkCommandBuffer>& command_buffers);
};

