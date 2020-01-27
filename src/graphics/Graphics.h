#ifndef VULK_GRAPHICS_H
#define VULK_GRAPHICS_H

#include "Vulkan.h"
#include "Camera.h"
#include "../../VulkanMemoryAllocator/src/vk_mem_alloc.h"
#include "BufferAllocation.h"
#include "RenderComponent.h"
#include "../curves/Bspline.h"
#include "../springsystem/Spring.h"
#include "../springsystem/SpringSystem.h"

#define UP glm::vec3(0.0f, 0.0f, 1.0f)
#define FORWARD glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)

class Graphics {
public:
	Graphics();

	void init();
	void initData();
	void drawFrame();
	void wait();
	void clear();
	void cleanup();

	void regObject(RenderComponent *rObj);
	void deregObject(RenderComponent *rObj);

	void regSpring(Spring *spring);
	void deregSpring(Spring *spring);

	void setSSystems();

	Camera* getCamera();
	GLFWwindow* window;

private:
	Vulkan vulk;
	Camera camera;
	VmaAllocator allocator;

	void setCamera();
	BufferAllocation* allocate(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
	void upload(BufferAllocation *allocation, VkDeviceSize size, void *data);
	void upload(BufferAllocation *allocation, VkDeviceSize size, void *data, VkDeviceSize offset);
};


#endif //VULK_GRAPHICS_H
