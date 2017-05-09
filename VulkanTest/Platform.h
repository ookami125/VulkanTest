#pragma once

#include "BUILD_OPTIONS.h"
#include <stdint.h>
#include <vector>

void InitPlatform();
void DeInitPlatform();
void AddRequiredPlatformInstanceExtensions(std::vector<const char *> *instance_extensions);


#if BUILD_USE_GLFW

// Define as a build option 
#define USE_FRAMEWORK_GLFW 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// For Windows Message Box
#if defined( _WIN32 )
#undef APIENTRY
#include <windows.h>
#endif 

// WINDOWS
#elif defined(_WIN32)

#define VK_USE_PLATFORM_WIN32_KHR 1
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#include <Windows.h>

#elif defined(__linux)

#define VK_USE_PLATFORM_XCB_KHR 1
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME
#include <xcb\xcb.h>

#else
#error Platform not yet supported!
#endif

#include <vulkan\vulkan.h>
