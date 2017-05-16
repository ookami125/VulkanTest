#pragma once
#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include <vector>
#include <cstdlib>
#include <assert.h>
#include <windows.h>
#include "Shared.h"
#include "Console.h"

class Window;

class Renderer
{
private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_GPU = VK_NULL_HANDLE;
	VkQueue  m_Queue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_GPUProperties{};
	VkPhysicalDeviceMemoryProperties m_GPUMemoryProperties{};

	uint32_t m_GraphicsFamilyIndex = 0;

	Window* m_Window = nullptr;

	std::vector<std::string> m_PipelineNames;

	std::vector<const char*> m_InstanceLayers;
	std::vector<const char*> m_InstanceExtensions;

	VkDebugReportCallbackEXT m_DebugReport = VK_NULL_HANDLE;

	VkDebugReportCallbackCreateInfoEXT m_DebugReportCallbackCreateInfo = {};

public:
	Renderer(const std::vector<std::string> & used_pipeline_names);
	~Renderer();

	Window* InitWindow(uint32_t width, uint32_t height, char* name);
	bool Run();

	const VkInstance GetVulkanInstance() const;
	const VkPhysicalDevice GetVulkanPhysicalDevice() const;
	const VkDevice GetVulkanDevice() const;
	const VkQueue GetVulkanQueue() const;
	const uint32_t GetVulkanGraphicsQueueFamilyIndex() const;
	const VkPhysicalDeviceProperties& GetVulkanPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties& GetVulkanPhysicalDeviceMemoryProperties() const;

	const std::vector<std::string>& GetPipelineNames();

private:
	void SetupLayersAndExtensions();

	void InitInstance();
	void DestroyInstance();

	void InitDevice();
	void DestroyDevice();

	void SetupDebug();
	void InitDebug();
	void DestroyDebug();
};