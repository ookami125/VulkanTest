#include "Shared.h"

#if BUILD_ENABLE_VULKAN_RUNTIME_DEBUG

void ErrorCheckFull(VkResult result, char* File, int Line)
{
	if (result < 0)
	{
		switch (result)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			printf("VK_ERROR_OUT_OF_HOST_MEMORY :");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			printf("VK_ERROR_OUT_OF_DEVICE_MEMORY :");
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			printf("VK_ERROR_INITIALIZATION_FAILED :");
			break;
		case VK_ERROR_DEVICE_LOST:
			printf("VK_ERROR_DEVICE_LOST :");
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			printf("VK_ERROR_MEMORY_MAP_FAILED :");
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			printf("VK_ERROR_LAYER_NOT_PRESENT :");
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			printf("VK_ERROR_EXTENSION_NOT_PRESENT :");
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			printf("VK_ERROR_FEATURE_NOT_PRESENT :");
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			printf("VK_ERROR_INCOMPATIBLE_DRIVER ::");
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			printf("VK_ERROR_TOO_MANY_OBJECTS :");
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			printf("VK_ERROR_FORMAT_NOT_SUPPORTED :");
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			printf("VK_ERROR_FRAGMENTED_POOL :");
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			printf("VK_ERROR_SURFACE_LOST_KHR :");
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			printf("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR :");
			break;
		case VK_SUBOPTIMAL_KHR:
			printf("VK_SUBOPTIMAL_KHR :");
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			printf("VK_ERROR_OUT_OF_DATE_KHR :");
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			printf("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR :");
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			printf("VK_ERROR_VALIDATION_FAILED_EXT :");
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			printf("VK_ERROR_INVALID_SHADER_NV :");
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
			printf("VK_ERROR_OUT_OF_POOL_MEMORY_KHR :");
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX:
			printf("VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX :");
			break;
		}
		printf_s("FILE:\"%s\" LINE:%d\n", File, Line);
		exit(-1);
	}
}

uint32_t FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties * gpuMemoryProperties, const VkMemoryRequirements * memoryRequirements, const VkMemoryPropertyFlags requiredProperties)
{
	for (uint32_t i = 0; i < gpuMemoryProperties->memoryTypeCount; ++i) {
		if (memoryRequirements->memoryTypeBits & (1 << i)) {
			if ((gpuMemoryProperties->memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties) {
				return i;
			}
		}
	}
	assert(0 && "Couldn't find proper memory type.");
	return UINT32_MAX;
}

#else
void ErrorCheckFull(VkResult result, char* File, int Line) {};
#endif