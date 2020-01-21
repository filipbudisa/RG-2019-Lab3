#include "BufferAllocation.h"

BufferAllocation::BufferAllocation(VkBuffer buffer, VmaAllocation allocation) : buffer(buffer), allocation(allocation){ }
