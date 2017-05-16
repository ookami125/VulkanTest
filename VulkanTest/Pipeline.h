#pragma once
#include "BUILD_OPTIONS.h"
#include "Platform.h"

#include "Shared.h"
#include "Renderer.h"
#include "Window.h"
#include "Mesh.h"

#include <string>
#include <fstream>
#include <vector>

class Window;
class Renderer;

// Pipeline handles vulkan pipelines, it's a relatively big object so it got it's own class
// This class automatically creates a vulkan pipeline from given shader sources and window
class Pipeline
{
public:
	Pipeline(Renderer * renderer, Window * window, const std::string & name);
	~Pipeline();

	VkPipeline GetVulkanPipeline();

	const std::string& GetName();

private:
	std::string m_Name;

	Renderer* m_Renderer = nullptr;
	Window* m_Window = nullptr;
	VkPhysicalDevice m_GPU = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueue m_Queue = VK_NULL_HANDLE;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	VkShaderModule m_ShaderModuleVertex = VK_NULL_HANDLE;
	VkShaderModule m_ShaderModuleFragment = VK_NULL_HANDLE;
};