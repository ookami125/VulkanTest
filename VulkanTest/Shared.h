#pragma once

#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include <assert.h>
#include <vector>

void ErrorCheckFull(VkResult result, char* File, int Line);

#define ErrorCheck(result) ErrorCheckFull(result, __FILE__, __LINE__)

uint32_t FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties * gpuMemoryProperties, const VkMemoryRequirements * memoryRequirements, const VkMemoryPropertyFlags requiredProperties);