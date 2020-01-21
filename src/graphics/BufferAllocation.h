#ifndef VULK_BUFFERALLOCATION_H
#define VULK_BUFFERALLOCATION_H


#include <vulkan/vulkan.h>
#include "../../VulkanMemoryAllocator/src/VmaUsage.h"

class BufferAllocation {
public:
	BufferAllocation(VkBuffer buffer, VmaAllocation allocation);

	VkBuffer buffer;
	VmaAllocation allocation;
};


#endif //VULK_BUFFERALLOCATION_H
