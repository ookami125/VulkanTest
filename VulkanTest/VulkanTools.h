#pragma once
#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "Shared.h"
#include "Renderer.h"
#include <vector>

struct Buffer
{
	VkBuffer					buffer;
	VkDeviceMemory				memory;
	VkMemoryRequirements		memory_requirements;
	VkMemoryPropertyFlags		memory_properties;
	uint64_t					memory_size;			// in bytes
	uint64_t					memory_alignment;		// in bytes
	uint32_t					memory_type_id;
};


class Renderer;

void FindBufferMemoryType(Renderer * renderer, Buffer & buffer);

void AllocateBuffersMemory(Renderer * renderer, std::vector<Buffer> & buffers);
void FreeBuffersMemory(Renderer * renderer, std::vector<Buffer> & buffers);
