#include "Graphics.h"
#include "../storage/Storage.h"
#include "Vulkan.h"
#include "../Game.h"


void Graphics::init(){
	vulk.init();
	window = vulk.window;

	//initData();
}

void Graphics::initData(){
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = vulk.physicalDevice;
	allocatorInfo.device = vulk.device;
	vmaCreateAllocator(&allocatorInfo, &allocator);

	for(int i = 0; i < Storage::renderObjects.size(); i++){
		regObject(Storage::renderObjects[i]);
	}

	for(int i = 0; i < Storage::splines.size(); i++){
		regSpline(Storage::splines[i]);
	}

	for(int i = 0; i < Storage::springs.size(); i++){
		regSpring(Storage::springs[i]);
	}

	for(int i = 0; i < Storage::sSystems.size(); i++){
		regMass(Storage::sSystems[i]);
	}
}

void Graphics::initCompute(){
	vulk.prepareCompute();
}

void Graphics::doCompute(){
	vulk.doComputeSprings();
	vulk.doComputeMass();
}

void Graphics::regObject(RenderComponent *rObj){
	rObj->vertexBuffer = allocate(rObj->mesh.vertices.size() * sizeof(Vertex),
								  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	rObj->indexBuffer = allocate(rObj->mesh.indices.size() * sizeof(uint16_t),
								 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	upload(rObj->vertexBuffer, rObj->mesh.vertices.size() * sizeof(Vertex), rObj->mesh.vertices.data());
	upload(rObj->indexBuffer, rObj->mesh.indices.size() * sizeof(uint16_t), rObj->mesh.indices.data());
}

void Graphics::deregObject(RenderComponent *rObj){
	vmaDestroyBuffer(allocator, rObj->vertexBuffer->buffer, rObj->vertexBuffer->allocation);
	vmaDestroyBuffer(allocator, rObj->indexBuffer->buffer, rObj->indexBuffer->allocation);
}

void Graphics::regMass(SpringSystem *system){
	system->pointBuffer = allocate(system->glPoints.size() * sizeof(glMassPoint),
								  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	system->springBuffer = allocate(system->glSprings.size() * sizeof(glSpring),
								   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	upload(system->pointBuffer, system->glPoints.size() * sizeof(glMassPoint), system->glPoints.data());
	upload(system->springBuffer, system->glSprings.size() * sizeof(glSpring), system->glSprings.data());
}

void Graphics::deregMass(SpringSystem *system){
	vmaDestroyBuffer(allocator, system->pointBuffer->buffer, system->pointBuffer->allocation);
	vmaDestroyBuffer(allocator, system->springBuffer->buffer, system->pointBuffer->allocation);
}

void Graphics::regSpline(Bspline *spline){
	spline->vertexBuffer = allocate(spline->vertices.size() * sizeof(Vertex),
								  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	spline->directionBuffer = allocate(spline->direction.size() * sizeof(Vertex),
									VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	upload(spline->vertexBuffer, spline->vertices.size() * sizeof(Vertex), spline->vertices.data());
	upload(spline->directionBuffer, spline->direction.size() * sizeof(Vertex), spline->direction.data());
}

void Graphics::deregSpline(Bspline *spline){
	vmaDestroyBuffer(allocator, spline->vertexBuffer->buffer, spline->vertexBuffer->allocation);
	vmaDestroyBuffer(allocator, spline->directionBuffer->buffer, spline->directionBuffer->allocation);
}

void Graphics::regSpring(Spring *spring){
	spring->vertexBuffer = allocate(2 * sizeof(Vertex),
									VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	upload(spring->vertexBuffer, spring->vertices.size() * sizeof(Vertex), spring->vertices.data());
}

void Graphics::deregSpring(Spring *spring){
	vmaDestroyBuffer(allocator, spring->vertexBuffer->buffer, spring->vertexBuffer->allocation);
}

BufferAllocation *Graphics::allocate(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage){
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size; // the size of the buffer in bytes
	bufferInfo.usage = bufferUsage;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryUsage;

	VmaAllocation allocation;
	VkBuffer buffer;
	vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

	return new BufferAllocation(buffer, allocation);
}

void Graphics::upload(BufferAllocation *allocation, VkDeviceSize size, void *data){
	Graphics::upload(allocation, size, data, 0);
}

/**
 * Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
 * They can be used to store vertex data, which this one does.
 *
 * Unlike the Vulkan objects we've been dealing with so far, buffers do not automatically allocate memory for
 * themselves.
 *
 * https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 *
 * We're now going to change createVertexBuffer to only use a host visible buffer as temporary buffer and use a device
 * local one as actual vertex buffer.
 *
 * https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
 */
void Graphics::upload(BufferAllocation *allocation, VkDeviceSize size, void *data, VkDeviceSize offset){
	/**
	 * We're now using a new stagingBuffer with stagingBufferMemory for mapping and copying the vertex data. In this
	 * chapter we're going to use two new buffer usage flags:
	 * - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
	 * - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
	 */
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vulk.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 stagingBuffer, stagingBufferMemory);

	/**
	 * Fill the buffer
	 *
	 * This is done by mapping the buffer memory into CPU accessible memory.
	 *
	 * This function allows us to access a region of the specified memory resource defined by an offset and size. The
	 * offset and size here are 0 and bufferInfo.size, respectively. It is also possible to specify the special value
	 * VK_WHOLE_SIZE to map all of the memory. The second to last parameter can be used to specify flags, but there
	 * aren't any available yet in the current API. It must be set to the value 0. The last parameter specifies the
	 * output for the pointer to the mapped memory.
	 *
	 * Simply memcpy the vertex data to the mapped memory and unmap it again
	 *
	 * Unfortunately the driver may not immediately copy the data into the buffer memory, for example because of
	 * caching. It is also possible that writes to the buffer are not visible in the mapped memory yet. There are two
	 * ways to deal with that problem:
	 * - Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	 * - Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges
	 * 	before reading from the mapped memory
	 * 	We went for the first approach, which ensures that the mapped memory always matches the contents of the
	 * 	allocated memory. Do keep in mind that this may lead to slightly worse performance than explicit flushing, but
	 * 	we'll see why that doesn't matter in the next chapter.
	 *
	 * 	Flushing memory ranges or using a coherent memory heap means that the driver will be aware of our writes to the
	 * 	buffer, but it doesn't mean that they are actually visible on the GPU yet. The transfer of data to the GPU is
	 * 	an operation that happens in the background and the specification simply tells us that it is guaranteed to be
	 * 	complete as of the next call to vkQueueSubmit.
	 */
	void *stagingData;

	/**
	 * The vertexBuffer is now allocated from a memory type that is device local, which generally means that we're not
	 * able to use vkMapMemory. However, we can copy data from the stagingBuffer to the vertexBuffer. We have to
	 * indicate that we intend to do that by specifying the transfer source flag for the stagingBuffer and the transfer
	 * destination flag for the vertexBuffer, along with the vertex buffer usage flag.
	 */
	vkMapMemory(vulk.device, stagingBufferMemory, 0, size, 0, &stagingData);
	memcpy(stagingData, data, (size_t) size);
	vkUnmapMemory(vulk.device, stagingBufferMemory);

	vulk.copyBuffer(stagingBuffer, allocation->buffer, offset, size);
	vkDestroyBuffer(vulk.device, stagingBuffer, nullptr);
	vkFreeMemory(vulk.device, stagingBufferMemory, nullptr);

	/**
	 * It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	 * for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	 * maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	 * NVIDIA GTX 1080.
	 *
	 * The right way to allocate memory for a large number of objects at the same time is to create a custom allocator
	 * that splits up a single allocation among many different objects by using the offset parameters that we've seen
	 * in many functions.
	 *
	 * You can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the
	 * GPUOpen initiative.
	 *
	 * https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
	 */

	/**
	 * Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a
	 * single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers.
	 *
	 * It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the
	 * same render operations, provided that their data is refreshed, of course. This is known as aliasing and some
	 * Vulkan functions have explicit flags to specify that you want to do this.
	 *
	 * https://developer.nvidia.com/vulkan-memory-management
	 */
}

void Graphics::drawFrame(){
	Storage::clearGarbage();
	setCamera();
	setParticles();
	//setSSystems();
	vulk.drawFrame();
}

void Graphics::wait(){
	// Wait to finish processing
	vkDeviceWaitIdle(vulk.device);
}

void Graphics::clear(){
	vmaDestroyAllocator(allocator);
}

void Graphics::cleanup(){
	// Wait to finish processing
	vkDeviceWaitIdle(vulk.device);

	clear();

	vulk.cleanup();
}


void Graphics::setCamera(){
	//std::lock_guard<std::mutex> lock(*vulk.uboMutex);

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), -glm::vec3(camera.getX(), camera.getY(), camera.getZ()));

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 direction = camera.getDirection();

	vulk.uniformBufferObjects.viewProj.view = glm::lookAt(position, direction, UP) * translate;
}

void Graphics::setParticles(){
	glm::vec3 eye = camera.getPosition();

	for(Particle* p : Storage::particles){ // Dorian Brmbota
		WorldObject* wObj = p->getWObject();
		glm::vec3 pLoc = wObj->position;

		glm::vec3 rot = glm::normalize(eye - pLoc);
		glm::quat rotation = glm::rotation(wObj->renderComponent->mesh.vertices[0].normal, rot);

		rot *= sin(p->getRotation()/2);
		glm::quat rotation2(cos(p->getRotation()/2), rot);

		wObj->rotation = rotation2 * rotation;

		wObj->setTransformation();
	}
}

void Graphics::setSSystems(){
	if(drawMesh){
		for(Spring *spring : Storage::springs){
			upload(spring->vertexBuffer, spring->vertices.size() * sizeof(Vertex), spring->vertices.data());
		}
	}

	for(SpringSystem* system : Storage::sSystems){
		RenderComponent* rObj = system->object->renderComponent;
		// todo: glVertex format prije uploada
		//upload(rObj->vertexBuffer, rObj->mesh.vertices.size() * sizeof(Vertex), rObj->mesh.vertices.data());
		//upload(system->pointBuffer, system->glPoints.size() * sizeof(glMassPoint), system->glPoints.data());

		for(int i = 0; i < system->glPoints.size(); i++){
			upload(system->pointBuffer, sizeof(glm::vec4), &system->glPoints[i].force, i * sizeof(glMassPoint));
		}
	}
}

Camera* Graphics::getCamera(){
	return &camera;
}

Graphics::Graphics() : vulk(), camera(3.0f, 3.0f, 4.0f, 0.4f, 4.0f, 0.0f){ }

