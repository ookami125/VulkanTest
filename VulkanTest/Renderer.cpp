#include "Renderer.h"
#include "Window.h"

Renderer::Renderer()
{
	SetupLayersAndExtensions();
	SetupDebug();
	InitInstance();
	InitDebug();
	InitDevice();
}

Renderer::~Renderer()
{
	delete m_Window;
	DestroyDevice();
	DestroyDebug();
	DestroyInstance();
}

Window * Renderer::InitWindow(uint32_t width, uint32_t height, char * name)
{
	m_Window = new Window(this, width, height, name);
	return m_Window;
}

bool Renderer::Run()
{
	if (m_Window != nullptr)
	{
		return m_Window->Update();
	}
	return true;
}

const VkInstance Renderer::GetVulkanInstance() const
{
	return m_Instance;
}

const VkPhysicalDevice Renderer::GetVulkanPhysicalDevice() const
{
	return m_GPU;
}

const VkDevice Renderer::GetVulkanDevice() const
{
	return m_Device;
}

const VkQueue Renderer::GetVulkanQueue() const
{
	return m_Queue;
}

const uint32_t Renderer::GetVulkanGraphicsQueueFamilyIndex() const
{
	return m_GraphicsFamilyIndex;
}

const VkPhysicalDeviceProperties & Renderer::GetVulkanPhysicalDeviceProperties() const
{
	return m_GPUProperties;
}

const VkPhysicalDeviceMemoryProperties & Renderer::GetVulkanPhysicalDeviceMemoryProperties() const
{
	return m_GPUMemoryProperties;
}

void Renderer::SetupLayersAndExtensions()
{
	//m_InstanceLayers.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	m_InstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	AddRequiredPlatformInstanceExtensions(&m_InstanceExtensions);
}

void Renderer::InitInstance()
{
	//Just setting some application related settings
	VkApplicationInfo applicationInfo {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1,0,46);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = "Vulkan Test";

	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = m_InstanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_InstanceLayers.data();
	instanceCreateInfo.enabledExtensionCount = m_InstanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.data();
	instanceCreateInfo.pNext = &m_DebugReportCallbackCreateInfo;
	
	//This is similar to opengl context (I'm told).
	ErrorCheck(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m_Instance));
}

void Renderer::DestroyInstance()
{
	vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
	m_Instance = VK_NULL_HANDLE;
}

void Renderer::InitDevice()
{
	{
		//Count all the devices that support Vulkan.
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, VK_NULL_HANDLE);

		//put all those devices into a fucking list cause it cant do it in one fucking go.
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		//TODO: Grab a better gpu first isnt always best.
		m_GPU = devices[0];

		vkGetPhysicalDeviceProperties(m_GPU, &m_GPUProperties);
		vkGetPhysicalDeviceMemoryProperties(m_GPU, &m_GPUMemoryProperties);
	}
	{
		uint32_t familyCount = 0;
		//Get the type of shit I can send to the gpu
		vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, VK_NULL_HANDLE);
		std::vector<VkQueueFamilyProperties> families(familyCount);

		//I could bitch more but I wont
		vkGetPhysicalDeviceQueueFamilyProperties(m_GPU, &familyCount, families.data());

		bool found = false;
		for (uint32_t i=0; i<families.size(); ++i)
		{
			if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				found = true;
				m_GraphicsFamilyIndex = i;
				break;
			}
		}
		if (!found)
		{
			assert(0 && "Vulkan Error: Physical device selected doesnt support graphics.");
			std::exit(-1);
		}
	}

	{
		uint32_t propertyCount;
		vkEnumerateInstanceLayerProperties(&propertyCount, VK_NULL_HANDLE);
		std::vector<VkLayerProperties> properties(propertyCount);
		vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());

		printf_s("Instance Layers:\n");
		for (auto &i : properties)
		{
			bool found = false;
			for (const char* layer : m_InstanceLayers)
			{
				if (strcmp(layer, i.layerName) == 0)
				{
					found = true;
					break;
				}
			}
			if(found)
				Console::setColor(Console::GREEN);
			printf_s("  %-36s\t| %s\n", i.layerName, i.description);
			Console::setColor(Console::GREY);
		}
		printf("\n");
	}

	float queuePriorities[] = {1.0f};

	VkDeviceQueueCreateInfo deviceQueueCreateInfo {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = m_GraphicsFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCreateInfo {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	ErrorCheck(vkCreateDevice(m_GPU, &deviceCreateInfo, VK_NULL_HANDLE, &m_Device));

	vkGetDeviceQueue(m_Device, m_GraphicsFamilyIndex, 0, &m_Queue);
}

void Renderer::DestroyDevice()
{
	vkDestroyDevice(m_Device, nullptr);
	m_Device = nullptr;
}

#if BUILD_ENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT obj_type,
	uint64_t src_obj,
	size_t location,
	int32_t msg_code,
	const char * layer_prefix,
	const char * msg,
	void* user_data)
{
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		Console::setColor(Console::GREY), printf("INFO: ");
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		Console::setColor(Console::YELLOW), printf("WARN: ");
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		Console::setColor(Console::LIGHTRED), printf("ERROR: ");
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		Console::setColor(Console::GREY), printf("DEBUG: ");
	printf("@[%s] Code %d: %s\n", layer_prefix, msg_code, msg);
	Console::setColor(Console::GREY);
	return false;
}

void Renderer::SetupDebug()
{
	m_DebugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	m_DebugReportCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
	m_DebugReportCallbackCreateInfo.flags =
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		0;
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	//m_InstanceLayers.push_back("VK_LAYER_GOOGLE_threading");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_parameter_validation");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_swapchain");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
	m_InstanceLayers.push_back("VK_LAYER_LUNARG_core_validation");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_monitor");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_object_tracker");
	//m_InstanceLayers.push_back("VK_LAYER_LUNARG_screenshot");
	m_InstanceLayers.push_back("VK_LAYER_GOOGLE_unique_objects");

	//m_InstanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");

	m_InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

void Renderer::InitDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
	if (fvkCreateDebugReportCallbackEXT == VK_NULL_HANDLE || fvkDestroyDebugReportCallbackEXT == VK_NULL_HANDLE)
	{
		assert(0 && "Vulkan Error: Could not fetch debug report callback pointers");
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(m_Instance, &m_DebugReportCallbackCreateInfo, VK_NULL_HANDLE, &m_DebugReport);
}

void Renderer::DestroyDebug()
{
	fvkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, VK_NULL_HANDLE);
}

#else
void Renderer::SetupDebug() {};
void Renderer::InitDebug() {};
void Renderer::DestroyDebug() {};
#endif