#include "Vulkan.h"
#include "../utils.h"
#include "../storage/Storage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include "../Game.h"

void Vulkan::init(){
	initMisc();
	initWindow();
	initVulkan();
}

void Vulkan::initMisc(){
	running = new bool(true);

	uniformBufferObjects.viewProj.proj = glm::perspective(glm::radians(45.0f), WIDTH / (float) HEIGHT, 0.1f, 10.0f);
	uniformBufferObjects.viewProj.proj[1][1] *= -1;
}

void Vulkan::cleanupMisc(){
	*running = false;
}

void Vulkan::initWindow(){
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanTest", nullptr, nullptr);
	//glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

/**
 * The reason that we're creating a static function as a callback is because GLFW does not know how to properly call a
 * member function with the right this pointer to our HelloTriangleApplication instance.
 *
 * However, we do get a reference to the GLFWwindow in the callback and there is another GLFW function that allows you
 * to store an arbitrary pointer inside of it: glfwSetWindowUserPointer
 *
 * @param window
 * @param width
 * @param height
 */
void Vulkan::framebufferResizeCallback(GLFWwindow* window, int width, int height){
	auto app = reinterpret_cast<Vulkan*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Vulkan::initVulkan(){
	// Setup
	createInstance();
	setupDebugMessenger();

	// Presentation
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();

	// Vulkan pipeline
	createRenderPass();
	createDescriptorSetLayouts();
	createGraphicsPipeline();

	// Drawing
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	//createVertexBuffer(); // not in recreate
	//createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

/**
 * It is possible for the window surface to change such that the swap chain is no longer compatible with it. One of the
 * reasons that could cause this to happen is the size of the window changing. We have to catch these events and
 * recreate the swap chain.
 *
 * The disadvantage of this approach is that we need to stop all rendering before creating the new swap chain. It is
 * possible to create a new swap chain while drawing commands on an image from the old swap chain are still in-flight.
 * You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR struct and
 * destroy the old swap chain as soon as you've finished using it.
 */
void Vulkan::recreateSwapChain() {
	/**
	 * There is another case where a swap chain may become out of data and that is a special kind of window resizing:
	 * window minimization. This case is special because it will result in a frame buffer size of 0. We will handle
	 * that by pausing until the window is in the foreground again.
	 */
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	// we shouldn't touch resources that may still be in use
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void Vulkan::cleanup(){

	// Start clearGarbage
	cleanupSwapChain();


	vkDestroyDescriptorSetLayout(device, descriptorSets[0].layout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSets[1].layout, nullptr);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if(enableValidationLayers){
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();

	cleanupMisc();
}

void Vulkan::cleanupSwapChain(){
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	for(VkPipeline pipeline : graphicsPipelines){
		vkDestroyPipeline(device, pipeline, nullptr);
	}
	for(VkPipelineLayout layout : pipelineLayouts){
		vkDestroyPipelineLayout(device, layout, nullptr);
	}
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vkDestroyBuffer(device, uniformBuffers[0].buffer[i], nullptr);
		vkFreeMemory(device, uniformBuffers[0].memory[i], nullptr);

		vkDestroyBuffer(device, uniformBuffers[1].buffer[i], nullptr);
		vkFreeMemory(device, uniformBuffers[1].memory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

/**
 * The drawFrame function will perform the following operations:
 * - Acquire an image from the swap chain
 * - Execute the command buffer with that image as attachment in the framebuffer
 * - Return the image to the swap chain for presentation
 *
 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
 */
void Vulkan::drawFrame(){
	// Wait for last frame to finish
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	/**
	 * The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we
	 * wish to acquire an image. The third parameter specifies a timeout in nanoseconds for an image to become
	 * available. Using the maximum value of a 64 bit unsigned integer disables the timeout.
	 *
	 * The next two parameters specify synchronization objects that are to be signaled when the presentation engine
	 * is finished using the image. That's the point in time where we can start drawing to it. It is possible to
	 * specify a semaphore, fence or both. We're going to use our imageAvailableSemaphore for that purpose here.
	 *
	 * The last parameter specifies a variable to output the index of the swap chain image that has become
	 * available. The index refers to the VkImage in our swapChainImages array. We're going to use that index to
	 * pick the right command buffer.
	 */
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(),
						  imageAvailableSemaphores[currentFrame],
						  VK_NULL_HANDLE, &imageIndex);

	/**
	 * VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for
	 * rendering. Usually happens after a window resize.
	 *
	 * VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present to the surface, but the surface
	 * properties are no longer matched exactly.
	 */

	if(result == VK_ERROR_OUT_OF_DATE_KHR){
		// Although many drivers and platforms trigger VK_ERROR_OUT_OF_DATE_KHR automatically after a window resize,
		// it is not guaranteed to happen.
		recreateSwapChain();
		return;
	}else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Update the unofmr buffer (MVP matrices)
	updateUniformBuffer(imageIndex);

	// Record the command buffer
	recordCommandBuffer(imageIndex);

	// Submit the command buffer
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// Which command buffers to actually submit for execution
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// Which semaphores to signal once the command buffer(s) have finished execution
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Lock frame
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// Submit the command buffer to the graphics queue

	try{
		if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS){
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}catch(const std::exception& e){
		printf("foo\n");
	}

	// Presentation

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// Which semaphores to wait on before presentation can happen
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// Swap chains to present images to and the index of the image for each swap chain. This will almost always be
	// a single one.
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	/**
	 * There is one last optional parameter called pResults. It allows you to specify an array of VkResult values
	 * to check for every individual swap chain if presentation was successful. It's not necessary if you're only
	 * using a single swap chain, because you can simply use the return value of the present function.
	 */
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}


	// vkQueueWaitIdle(presentQueue);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::updateUniformBuffer(uint32_t currentImage){
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// ViewProj UBO

	uniformBufferObjects.viewProj.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
	uniformBufferObjects.viewProj.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, uniformBuffers[0].memory[currentImage], 0, sizeof(UBOViewProj), 0, &data);
	memcpy(data, &uniformBufferObjects.viewProj, sizeof(UBOViewProj));
	vkUnmapMemory(device, uniformBuffers[0].memory[currentImage]);

	// Lights UBO

	//uniformBufferObjects.lights.lights[0].pos = glm::vec3(2*cos(time), 2*sin(time), 2.0f);
	uniformBufferObjects.lights.lights[0].pos = glm::vec3(1.5f, 1.0f, 3.0f);
	uniformBufferObjects.lights.lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);

	vkMapMemory(device, uniformBuffers[1].memory[currentImage], 0, sizeof(UBOLights), 0, &data);
	memcpy(data, &uniformBufferObjects.lights, sizeof(UBOLights));
	vkUnmapMemory(device, uniformBuffers[1].memory[currentImage]);
}

void Vulkan::createInstance(){
	if(enableValidationLayers){
		if(!checkValidationLayerSupport()){
			throw std::runtime_error("validation layers requested, but not available!");
		}else{
			std::cout << "Validation layers enabled." << std::endl;
		}
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if(enableValidationLayers){
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}else{
		createInfo.enabledLayerCount = 0;
	}

	if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
		throw std::runtime_error("failed to create instance!");
	}
}

bool Vulkan::checkValidationLayerSupport(){
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector <VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for(const char *layerName : validationLayers){
		bool layerFound = false;

		for(const auto &layerProperties : availableLayers){
			if(strcmp(layerName, layerProperties.layerName) == 0){
				layerFound = true;
				break;
			}
		}

		if(!layerFound){
			return false;
		}
	}

	return true;
}

/**
 * Get extensions that physical device must have to be chosen
 * @return
 */
std::vector<const char *> Vulkan::getRequiredExtensions(){
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(enableValidationLayers){
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Vulkan::setupDebugMessenger(){
	if(!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData){

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void Vulkan::createSurface(){
	if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
		throw std::runtime_error("failed to create window surface!");
	}
}

void Vulkan::pickPhysicalDevice(){
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if(deviceCount == 0){
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector <VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for(const auto &device : devices){
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		std::cout << "Device: " << deviceProperties.deviceName << std::endl;

		if(isDeviceSuitable(device)){
			physicalDevice = device;
			//break;
		}
	}

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	std::cout << "Picked " << deviceProperties.deviceName << std::endl;

	if(physicalDevice == VK_NULL_HANDLE){
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool Vulkan::isDeviceSuitable(VkPhysicalDevice device){
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	deviceFeatures.samplerAnisotropy = VK_TRUE;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if(extensionsSupported){
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return /*deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		   &&*/ deviceFeatures.geometryShader
		   && findQueueFamilies(device).isComplete()
		   && extensionsSupported && swapChainAdequate
		   && deviceFeatures.samplerAnisotropy;
}

bool Vulkan::checkDeviceExtensionSupport(VkPhysicalDevice device){
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector <VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set <std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for(const auto &extension : availableExtensions){
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

/**
 * Create logical device with one Graphics and  one Present queue family. Also retrieves the queue families.
 */
void Vulkan::createLogicalDevice(){
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector <VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set <uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;

	for(uint32_t queueFamily : uniqueQueueFamilies){
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if(enableValidationLayers){
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}else{
		createInfo.enabledLayerCount = 0;
	}

	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

/**
 * Swap chain - infrastructure that owns the buffers we will render to before we visualize them on the screen.
 * Essentially a queue of images that are waiting to be presented to the screen. How exactly the queue works and the
 * conditions for presenting an image from the queue depend on how the swap chain is set up, but the general purpose of
 * the swap chain is to synchronize the presentation of images with the refresh rate of the screen.
 */
void Vulkan::createSwapChain(){
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if(swapChainSupport.capabilities.maxImageCount > 0 &&
	   imageCount > swapChainSupport.capabilities.maxImageCount){
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if(indices.graphicsFamily != indices.presentFamily){
		// throw std::runtime_error("graphics family queue and present family queue aren't same");

		/**
		 * VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership
		 * transfers.
		 */
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}else{
		/**
		 * VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly
		 * transfered before using it in another queue family. This option offers the best performance.
		 */
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS){
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

/**
 * Check if device supports Swap Chains:
 *  - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
 *  - Surface formats (pixel format, color space)
 *  - Available presentation modes
 * @param device
 * @return
 */
SwapChainSupportDetails Vulkan::querySwapChainSupport(VkPhysicalDevice device){
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if(formatCount != 0){
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if(presentModeCount != 0){
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
												  details.presentModes.data());
	}

	return details;
}

/**
 * Swap chain surface format (color depth)
 * @param availableFormats
 * @return
 */
VkSurfaceFormatKHR Vulkan::chooseSwapSurfaceFormat(const std::vector <VkSurfaceFormatKHR> &availableFormats){
	if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED){
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for(const auto &availableFormat : availableFormats){
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
		   availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
			return availableFormat;
		}
	}

	// throw std::runtime_error("preferred swap surface format not available");
	return availableFormats[0];
}


/**
 * Swap chain presentation mode (conditions for "swapping" images to the screen). There are four possible modes
 * available in Vulkan:
 *
 * - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away,
 * 	which may result in tearing.
 *
 * - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue
 * 	when the display is refreshed and the program inserts rendered images at the back of the queue. If the queue is full
 * 	then the program has to wait. This is most similar to vertical sync as found in modern games. The moment that the
 * 	display is refreshed is known as "vertical blank".
 *
 * - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the
 * 	queue was empty at the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred
 * 	right away when it finally arrives. This may result in visible tearing.
 *
 * - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application
 * 	when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be
 * 	used to implement triple buffering, which allows you to avoid tearing with significantly less latency issues than
 * 	standard vertical sync that uses double buffering.
 *
 * @param availablePresentModes
 * @return
 */
VkPresentModeKHR Vulkan::chooseSwapPresentMode(const std::vector <VkPresentModeKHR> availablePresentModes){
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for(const auto &availablePresentMode : availablePresentModes){
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR){
			return availablePresentMode;
		}else if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR){
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

/**
 * Swap chain extent (resolution of images in swap chain)
 * @param capabilities
 * @return
 */
VkExtent2D Vulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities){
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
		return capabilities.currentExtent;
	}else{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width,
									  std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
									   std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

/**
 * To use any VkImage, including those in the swap chain, in the render pipeline we have to create a VkImageView object.
 * An image view is quite literally a view into an image. It describes how to access the image and which part of the
 * image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels.
 */
void Vulkan::createImageViews(){
	swapChainImageViews.resize(swapChainImages.size());

	for(size_t i = 0; i < swapChainImages.size(); i++){
		/*VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS){
			throw std::runtime_error("failed to create image views!");
		}*/

		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

/**
 * Create a "render pass" object - we need to tell Vulkan about the framebuffer attachments that will be used while
 * rendering. We need to specify how many color and depth buffers there will be, how many samples to use for each of
 * them and how their contents should be handled throughout the rendering operations.
 *
 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes
 */
void Vulkan::createRenderPass(){
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	// What to do before rendirng and what to do after rendering to buffer
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// Stencil buffer
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Layout of the pixels in memory
	/**
	 * VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
	 * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
	 * VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
	 */
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpasses and attachment references
	/**
	 * Subpasses, ex. post-processing layers
	 */

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// https://vulkan-tutorial.com/Depth_buffering
	// # Depth attachment
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Subpass dependencies

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Render pass
	/**
	 * The render pass object can then be created by filling in the VkRenderPassCreateInfo structure with an array
	 * of attachments and subpasses. The VkAttachmentReference objects reference attachments using the indices of
	 * this array.
	 */

	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
		throw std::runtime_error("failed to create render pass!");
	}
}

/**
 * Model view projection matrices passing
 *
 * https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
 */
void Vulkan::createDescriptorSetLayouts(){
	descriptorSets.resize(2);

	createDescriptorSetLayout(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &descriptorSets[0].layout);
	createDescriptorSetLayout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &descriptorSets[1].layout);
}

void Vulkan::createDescriptorSetLayout(VkShaderStageFlags stageFlags, VkDescriptorType descriptorType, uint32_t binding, VkDescriptorSetLayout *layout){
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = stageFlags;
	layoutBinding.pImmutableSamplers = nullptr; // Optional, only relevant for image sampling related descriptors

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

/**
 * The graphics pipeline is the sequence of operations that take the vertices and textures of your meshes all the way
 * to the pixels in the render targets:
 *
 * The input assembler collects the raw vertex data from the buffers you specify and may also use an index buffer to
 * repeat certain elements without having to duplicate the vertex data itself.
 *
 * The vertex shader is run for every vertex and generally applies transformations to turn vertex positions from model
 * space to screen space. It also passes per-vertex data down the pipeline.
 *
 * The tessellation shaders allow you to subdivide geometry based on certain rules to increase the mesh quality. This
 * is often used to make surfaces like brick walls and staircases look less flat when they are nearby.
 *
 * The geometry shader is run on every primitive (triangle, line, point) and can discard it or output more primitives
 * than came in. This is similar to the tessellation shader, but much more flexible. However, it is not used much in
 * today's applications because the performance is not that good on most graphics cards except for Intel's integrated
 * GPUs.
 *
 * The rasterization stage discretizes the primitives into fragments. These are the pixel elements that they fill on
 * the framebuffer. Any fragments that fall outside the screen are discarded and the attributes outputted by the vertex
 * shader are interpolated across the fragments, as shown in the figure. Usually the fragments that are behind other
 * primitive fragments are also discarded here because of depth testing.
 *
 * The fragment shader is invoked for every fragment that survives and determines which framebuffer(s) the fragments
 * are written to and with which color and depth values. It can do this using the interpolated data from the vertex
 * shader, which can include things like texture coordinates and normals for lighting.
 *
 * The color blending stage applies operations to mix different fragments that map to the same pixel in the framebuffer.
 * Fragments can simply overwrite each other, add up or be mixed based upon transparency.
 *
 * -----
 *
 * Fixed-function stages: input assembly, rasterization, color blending
 * These stages allow you to tweak their operations using parameters, but the way they work is predefined.
 *
 * Programmable stages: Vertex shader, Tessellation, Geometry shader, Fragment shader
 * You can upload your own code to the graphics card to apply exactly the operations you want. This allows you to use
 * fragment shaders, for example, to implement anything from texturing and lighting to ray tracers. These programs run
 * on many GPU cores simultaneously to process many objects, like vertices and fragments in parallel.
 *
 * -----
 *
 * Some of the programmable stages are optional based on what you intend to do. For example, the tessellation and
 * geometry stages can be disabled if you are just drawing simple geometry. If you are only interested in depth
 * values then you can disable the fragment shader stage, which is useful for shadow map generation.
 *
 * -----
 *
 * The graphics pipeline in Vulkan is almost completely immutable, so you must recreate the pipeline from scratch if
 * you want to change shaders, bind different framebuffers or change the blend function. The disadvantage is that you'll
 * have to create a number of pipelines that represent all of the different combinations of states you want to use in
 * your rendering operations. However, because all of the operations you'll be doing in the pipeline are known in
 * advance, the driver can optimize for it much better.
 */
void Vulkan::createGraphicsPipeline(){

	pipelineLayouts.resize(3);
	graphicsPipelines.resize(3);
	createTopoPipeline();
	createLinePipeline();
	createClothPipeline();

	// Pipeline creation
	/**
	 * The second parameter, for which we've passed the VK_NULL_HANDLE argument, references an optional
	 * VkPipelineCache object. A pipeline cache can be used to store and reuse data relevant to pipeline creation
	 * across multiple calls to vkCreateGraphicsPipelines and even across program executions if the cache is stored
	 * to a file. This makes it possible to significantly speed up pipeline creation at a later time.
	 */
	/*if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, pipelineInfos.size(), pipelineInfos.data(), nullptr, graphicsPipelines.data()) !=
	   VK_SUCCESS){
		throw std::runtime_error("failed to create graphics pipeline!");
	}*/

	// Destroy shader modules after pipeline is created
	/*vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);*/
}

void Vulkan::createTopoPipeline(){
	// Shader Stages
	/**
	 * Programmable
	 *
	 * Compiled shaders:
	 * http://www.mattikariluoma.com/blog/Segmentation%20Fault%20during%20vkCreateGraphicsPipelines.html
	 */

	auto vertShaderCode = readFile("shaders/topo.vert.spv");
	auto fragShaderCode = readFile("shaders/topo.frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	// Fixed fucntion state
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
	 */
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport & Scissoring

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


	// Multisampling

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Color Blending

	// # - no blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ONE
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ZERO
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// # - alpha blending
	/*colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	/** Dynamic State
	 *
	 * For changing features without recreating the pipeline.
	 * Viewport, line width, blend constants, etc.
	 */
	/*VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;*/

	// Push constants
	/**
	 * Push constants is a way to quickly provide a small amount of uniform data to shaders. It should be much quicker
	 * than UBOs but a huge limitation is the size of data - spec requires 128 bytes to be available for a push constant
	 * range. Hardware vendors may support more.
	 *
	 * Different shader stages of a given pipeline can use the same push constant block (similarly to UBOs) or smaller
	 * parts of the whole range. But, what is important, each shader stage can use only one push constant block. It can
	 * contain multiple members, though. Another important thing is that the total data size (across all shader stages
	 * which use push constants) must fit into the size constraint. So the constraint is not per stage but per whole
	 * range.
	 */
	VkPushConstantRange transformPushConst = {};
	transformPushConst.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	transformPushConst.size = sizeof(MeshTransforms);
	transformPushConst.offset = 0;

	std::vector<VkPushConstantRange> pushConstants = { transformPushConst };

	// Pipeline layout

	std::vector<VkDescriptorSetLayout> descSetLayouts = { descriptorSets[0].layout, descriptorSets[1].layout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descSetLayouts.size(); // Optional
	pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size(); // Optional
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data(); // Optional

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts[0]) != VK_SUCCESS){
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Pipeline info creation
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
	 */

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pTessellationState = nullptr;

	// Shader Stages
	pipelineInfo.pStages = shaderStages.data();

	// Fixed func state
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional

	// Depth and stencil state
	// https://vulkan-tutorial.com/Depth_buffering

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;

	// The depthWriteEnable field specifies if the new depth of fragments that pass the depth test should actually be
	// written to the depth buffer. This is useful for drawing transparent objects. They should be compared to the
	// previously rendered opaque objects, but not cause further away transparent objects to not be drawn.

	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	/**
	 * The depthBoundsTestEnable, minDepthBounds and maxDepthBounds fields are used for the optional depth bound test.
	 * Basically, this allows you to only keep fragments that fall within the specified depth range. We won't be using
	 * this functionality.
	 */

	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	pipelineInfo.pDepthStencilState = &depthStencil;

	// Layout
	pipelineInfo.layout = pipelineLayouts[0];

	// Render pass & subpass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	// Pipeline derivate
	/**
	 * The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much
	 * functionality in common with an existing pipeline and switching between pipelines from the same parent can
	 * also be done quicker.
	 */
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelines[0]) !=
	   VK_SUCCESS){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Vulkan::createLinePipeline(){
// Shader Stages
	/**
	 * Programmable
	 *
	 * Compiled shaders:
	 * http://www.mattikariluoma.com/blog/Segmentation%20Fault%20during%20vkCreateGraphicsPipelines.html
	 */

	auto vertShaderCode = readFile("shaders/line.vert.spv");
	auto fragShaderCode = readFile("shaders/line.frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	// Fixed fucntion state
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
	 */
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport & Scissoring

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;


	// Multisampling

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Color Blending

	// # - no blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ONE
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ZERO
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// # - alpha blending
	/*colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	/** Dynamic State
	 *
	 * For changing features without recreating the pipeline.
	 * Viewport, line width, blend constants, etc.
	 */
	/*VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;*/

	// Push constants
	/**
	 * Push constants is a way to quickly provide a small amount of uniform data to shaders. It should be much quicker
	 * than UBOs but a huge limitation is the size of data - spec requires 128 bytes to be available for a push constant
	 * range. Hardware vendors may support more.
	 *
	 * Different shader stages of a given pipeline can use the same push constant block (similarly to UBOs) or smaller
	 * parts of the whole range. But, what is important, each shader stage can use only one push constant block. It can
	 * contain multiple members, though. Another important thing is that the total data size (across all shader stages
	 * which use push constants) must fit into the size constraint. So the constraint is not per stage but per whole
	 * range.
	 */
	VkPushConstantRange transformPushConst = {};
	transformPushConst.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	transformPushConst.size = sizeof(MeshTransforms);
	transformPushConst.offset = 0;

	std::vector<VkPushConstantRange> pushConstants = { transformPushConst };

	// Pipeline layout

	std::vector<VkDescriptorSetLayout> descSetLayouts = { descriptorSets[0].layout, descriptorSets[1].layout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descSetLayouts.size(); // Optional
	pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size(); // Optional
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data(); // Optional

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts[1]) != VK_SUCCESS){
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Pipeline info creation
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
	 */

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pTessellationState = nullptr;

	// Shader Stages
	pipelineInfo.pStages = shaderStages.data();

	// Fixed func state
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional

	// Depth and stencil state
	// https://vulkan-tutorial.com/Depth_buffering

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;

	// The depthWriteEnable field specifies if the new depth of fragments that pass the depth test should actually be
	// written to the depth buffer. This is useful for drawing transparent objects. They should be compared to the
	// previously rendered opaque objects, but not cause further away transparent objects to not be drawn.

	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	/**
	 * The depthBoundsTestEnable, minDepthBounds and maxDepthBounds fields are used for the optional depth bound test.
	 * Basically, this allows you to only keep fragments that fall within the specified depth range. We won't be using
	 * this functionality.
	 */

	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	pipelineInfo.pDepthStencilState = &depthStencil;

	// Layout
	pipelineInfo.layout = pipelineLayouts[1];

	// Render pass & subpass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	// Pipeline derivate
	/**
	 * The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much
	 * functionality in common with an existing pipeline and switching between pipelines from the same parent can
	 * also be done quicker.
	 */
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelines[1]) !=
	   VK_SUCCESS){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Vulkan::createClothPipeline(){
// Shader Stages
	/**
	 * Programmable
	 *
	 * Compiled shaders:
	 * http://www.mattikariluoma.com/blog/Segmentation%20Fault%20during%20vkCreateGraphicsPipelines.html
	 */

	auto vertShaderCode = readFile("shaders/topo.vert.spv");
	auto fragShaderCode = readFile("shaders/topo.frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	// Fixed fucntion state
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
	 */
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport & Scissoring

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // back-face culling
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


	// Multisampling

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Color Blending

	// # - no blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ONE
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional - VK_BLEND_FACTOR_ZERO
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// # - alpha blending
	/*colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	/** Dynamic State
	 *
	 * For changing features without recreating the pipeline.
	 * Viewport, line width, blend constants, etc.
	 */
	/*VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;*/

	// Push constants
	/**
	 * Push constants is a way to quickly provide a small amount of uniform data to shaders. It should be much quicker
	 * than UBOs but a huge limitation is the size of data - spec requires 128 bytes to be available for a push constant
	 * range. Hardware vendors may support more.
	 *
	 * Different shader stages of a given pipeline can use the same push constant block (similarly to UBOs) or smaller
	 * parts of the whole range. But, what is important, each shader stage can use only one push constant block. It can
	 * contain multiple members, though. Another important thing is that the total data size (across all shader stages
	 * which use push constants) must fit into the size constraint. So the constraint is not per stage but per whole
	 * range.
	 */
	VkPushConstantRange transformPushConst = {};
	transformPushConst.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	transformPushConst.size = sizeof(MeshTransforms);
	transformPushConst.offset = 0;

	std::vector<VkPushConstantRange> pushConstants = { transformPushConst };

	// Pipeline layout

	std::vector<VkDescriptorSetLayout> descSetLayouts = { descriptorSets[0].layout, descriptorSets[1].layout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descSetLayouts.size(); // Optional
	pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size(); // Optional
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data(); // Optional

	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts[2]) != VK_SUCCESS){
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Pipeline info creation
	/**
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
	 */

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pTessellationState = nullptr;

	// Shader Stages
	pipelineInfo.pStages = shaderStages.data();

	// Fixed func state
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional

	// Depth and stencil state
	// https://vulkan-tutorial.com/Depth_buffering

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;

	// The depthWriteEnable field specifies if the new depth of fragments that pass the depth test should actually be
	// written to the depth buffer. This is useful for drawing transparent objects. They should be compared to the
	// previously rendered opaque objects, but not cause further away transparent objects to not be drawn.

	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	/**
	 * The depthBoundsTestEnable, minDepthBounds and maxDepthBounds fields are used for the optional depth bound test.
	 * Basically, this allows you to only keep fragments that fall within the specified depth range. We won't be using
	 * this functionality.
	 */

	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	pipelineInfo.pDepthStencilState = &depthStencil;

	// Layout
	pipelineInfo.layout = pipelineLayouts[2];

	// Render pass & subpass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	// Pipeline derivate
	/**
	 * The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much
	 * functionality in common with an existing pipeline and switching between pipelines from the same parent can
	 * also be done quicker.
	 */
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelines[2]) !=
	   VK_SUCCESS){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule Vulkan::createShaderModule(const std::vector<char> &code){
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

/**
 * Create frame buffers from (vkImageView -> vkImage) for the swap chain. The attachments specified during render pass
 * creation are bound by wrapping them into a VkFramebuffer object. A framebuffer object references all of the
 * VkImageView objects that represent the attachments. In our case that will be only a single one: the color attachment.
 * However, the image that we have to use for the attachment depends on which image the swap chain returns when we
 * retrieve one for presentation. That means that we have to create a framebuffer for all of the images in the swap
 * chain and use the one that corresponds to the retrieved image at drawing time.
 *
 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Framebuffers
 */
void Vulkan::createFramebuffers(){
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for(size_t i = 0; i < swapChainImageViews.size(); i++){
		std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i],
				depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS){
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

/**
 * We have to create a command pool before we can create command buffers. Command pools manage the memory that is used
 * to store the buffers and command buffers are allocated from them.
 *
 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers
 */
void Vulkan::createCommandPool(){
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS){
		throw std::runtime_error("failed to create command pool!");
	}
}

/**
 * https://vulkan-tutorial.com/Texture_mapping/Images
 *
 * @param width
 * @param height
 * @param format
 * @param tiling
 * @param usage
 * @param properties
 * @param image
 * @param imageMemory
 */
void Vulkan::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

/**
 * https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler
 *
 * @param image
 * @param format
 * @return
 */
VkImageView Vulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

/**
 * Function to record and execute vkCmdCopyBufferToImage to finish the job, but this command requires the image to be
 * in the right layout first.
 * @param image
 * @param format
 * @param oldLayout
 * @param newLayout
 */
void Vulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	/**
	 * One of the most common ways to perform layout transitions is using an image memory barrier. A pipeline barrier
	 * like that is generally used to synchronize access to resources, like ensuring that a write to a buffer completes
	 * before reading from it, but it can also be used to transition image layouts and transfer queue family ownership
	 * when VK_SHARING_MODE_EXCLUSIVE is used. There is an equivalent buffer memory barrier to do this for buffers.
	 */

	/**
	 * The first two fields specify layout transition. It is possible to use VK_IMAGE_LAYOUT_UNDEFINED as oldLayout if
	 * you don't care about the existing contents of the image.
	 */
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	/**
	 * If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of
	 * the queue families. They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default
	 * value!).
	 */
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	/**
	 * The image and subresourceRange specify the image that is affected and the specific part of the image. Our image
	 * is not an array and does not have mipmapping levels, so only one level and layer are specified.
	 */
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	/**
	 * Barriers are primarily used for synchronization purposes, so you must specify which types of operations that
	 * involve the resource must happen before the barrier, and which operations that involve the resource must wait on
	 * the barrier. We need to do that despite already using vkQueueWaitIdle to manually synchronize. The right values
	 * depend on the old and new layout, so we'll get back to this once we've figured out which transitions we're going
	 * to use.
	 */
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	// Solution to TODO
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	// https://vulkan-tutorial.com/Depth_buffering

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	// https://vulkan-tutorial.com/Texture_mapping/Images
	// # Layout transitions
	vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void Vulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height){
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
			width,
			height,
			1
	};

	vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
	);

	endSingleTimeCommands(commandBuffer);
}

/**
 * https://vulkan-tutorial.com/Depth_buffering
 */
void Vulkan::createDepthResources(){
	VkFormat depthFormat = findDepthFormat();
	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat Vulkan::findDepthFormat() {
	return findSupportedFormat(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}


/**
 * Make sure to use the VK_FORMAT_FEATURE_ flag instead of VK_IMAGE_USAGE_ in this case. All of these candidate formats
 * contain a depth component, but the latter two also contain a stencil component. We won't be using that yet, but we do
 * need to take that into account when performing layout transitions on images with these formats.
 *
 * Simple helper function that tells us if the chosen depth format contains a stencil component:
 */
bool Vulkan::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


VkFormat Vulkan::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

	/**
	 * The VkFormatProperties struct contains three fields:
	 * - linearTilingFeatures: Use cases that are supported with linear tiling
	 * - optimalTilingFeatures: Use cases that are supported with optimal tiling
	 * - bufferFeatures: Use cases that are supported for buffers
	 *
	 * Only the first two are relevant here, and the one we check depends on the tiling parameter of the function.
	 */

	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}


void Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
											VkBuffer& buffer, VkDeviceMemory& bufferMemory){
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size; // the size of the buffer in bytes
	bufferInfo.usage = usage;

	/**
	 * Just like the images in the swap chain, buffers can also be owned by a specific queue family or be shared
	 * between multiple at the same time. The buffer will only be used from the graphics queue, so we can stick to
	 * exclusive access.
	 */
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	/**
	 * The flags parameter is used to configure sparse buffer memory, which is not relevant right now. We'll leave it
	 * at the default value of 0.
	 */
	bufferInfo.flags = 0;

	if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
		throw std::runtime_error("failed to create vertex buffer!");
	}

	/**
	 * Allocate memory for buffer
	 *
	 * The VkMemoryRequirements struct has three fields:
	 * - size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
	 * - alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on bufferInfo.usage and bufferInfo.flags.
	 * - memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
	 */
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	/**
	 * The properties define special features of
	 * the memory, like being able to map it so we can write to it from the CPU. This property is indicated with
	 * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, but we also need to use the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property.
	 */
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	/**
	 * Associate the memory with the buffer
	 *
	 * The first three parameters are self-explanatory and the fourth parameter is the offset within the region of
	 * memory. Since this memory is allocated specifically for this the vertex buffer, the offset is simply 0. If
	 * the offset is non-zero, then it is required to be divisible by memRequirements.alignment.
	 */
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size){
	/**
	 * Memory transfer operations are executed using command buffers, just like drawing commands. Therefore we must
	 * first allocate a temporary command buffer.
	 */
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Copy:
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = offset; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	// Stop recording:
	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Vulkan::beginSingleTimeCommands() {
	/**
	 * Memory transfer operations are executed using command buffers, just like drawing commands. Therefore we must
	 * first allocate a temporary command buffer.
	 */

	// TODO: Create a separate command pool
	/**
	 * You may wish to create a separate command pool for these kinds of short-lived buffers, because the
	 * implementation may be able to apply memory allocation optimizations. You should use the
	 * VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
	 */
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	// Recording the command buffer:
	/**
	 * The VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT flag that we used for the drawing command buffers is not
	 * necessary here, because we're only going to use the command buffer once and wait with returning from the
	 * function until the copy operation has finished executing. It's good practice to tell the driver about our intent
	 * using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	 */
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Vulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	// Execute the command buffer:
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	/**
	 * There are again two possible ways to wait on this transfer to complete. We could use a fence and wait with
	 * vkWaitForFences, or simply wait for the transfer queue to become idle with vkQueueWaitIdle. A fence would allow
	 * you to schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at
	 * a time. That may give the driver more opportunities to optimize.
	 */
	vkQueueWaitIdle(graphicsQueue);

	// Freeing the command buffer:
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

/**
 *Graphics cards can offer different types of memory to allocate from. Each type of memory varies in terms of allowed
 * operations and performance characteristics. We need to combine the requirements of the buffer and our own
 * application requirements to find the right type of memory to use.
 *
 *
 * @param typeFilter The bit field of memory types that are suitable.
 * @param properties
 * @return
 */
uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){

	/**
	 * First we need to query info about the available types of memory using vkGetPhysicalDeviceMemoryProperties.
	 *
	 * The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes and memoryHeaps. Memory heaps are
	 * distinct memory resources like dedicated VRAM and swap space in RAM for when VRAM runs out. The different types
	 * of memory exist within these heaps. Right now we'll only concern ourselves with the type of memory and not the
	 * heap it comes from, but you can imagine that this can affect performance.
	 */
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	/**
	 * We also need to be able to write our vertex data to that memory. The memoryTypes array consists of VkMemoryType
	 * structs that specify the heap and properties of each type of memory.
	 */

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void Vulkan::createUniformBuffers(){
	uniformBuffers.resize(2);

	for(size_t i = 0; i < swapChainImages.size(); i++){
		createBuffer(sizeof(UBOViewProj), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 uniformBuffers[0].buffer[i], uniformBuffers[0].memory[i]);

		createBuffer(sizeof(UBOLights), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 uniformBuffers[1].buffer[i], uniformBuffers[1].memory[i]);
	}
}

/**
 * https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets
 */
void Vulkan::createDescriptorPool(){
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(2 * swapChainImages.size()); // TODO: descriptor count
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(2 * swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());;
	poolInfo.pPoolSizes = poolSizes.data();

	poolInfo.maxSets = static_cast<uint32_t>(4 * swapChainImages.size());

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Vulkan::createDescriptorSets(){
	createBufferDescriptorSet(descriptorSets[0].layout, descriptorSets[0].set, uniformBuffers[0], sizeof(UBOViewProj));
	createBufferDescriptorSet(descriptorSets[1].layout, descriptorSets[1].set, uniformBuffers[1], sizeof(UBOLights));
	//createImageDescriptorSet(descriptorSets[2].layout, descriptorSets[2].set, textureImageView, textureSampler);
}

void Vulkan::createBufferDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet *pos, UniformBuffer buffer, VkDeviceSize bufferSize){
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), layout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, pos) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer.buffer[i];
		bufferInfo.offset = 0;
		bufferInfo.range = bufferSize;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = pos[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

/**
 * Command buffers contain drawing commands for each frame.
 *
 * We can now start allocating command buffers and recording drawing commands in them. Because one of the drawing
 * commands involves binding the right VkFramebuffer, we'll actually have to record a command buffer for every image in
 * the swap chain once again.
 *
 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers
 */
void Vulkan::createCommandBuffers(){
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	/**
	 * The level parameter specifies if the allocated command buffers are primary or secondary command buffers:
	 *
	 * - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other
	 * command buffers.
	 *
	 * - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command
	 * buffers.
	 *
	 * We won't make use of the secondary command buffer functionality here, but you can imagine that it's helpful to
	 * reuse common operations from primary command buffers.
	 */

	if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS){
		throw std::runtime_error("failed to allocate command buffers!");
	}

	allocInfo.commandBufferCount = 1;

	if(vkAllocateCommandBuffers(device, &allocInfo, &computeCommandBuffer) != VK_SUCCESS){
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for(size_t i = 0; i < commandBuffers.size(); i++){
		//recordCommandBuffer(i);
	}
}

void Vulkan::recordCommandBuffer(size_t i){
	vkResetCommandBuffer(commandBuffers[i], 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	/**
	 * The flags parameter specifies how we're going to use the command buffer. The following values are available:
	 *
	 * - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it
	 * once.
	 *
	 * - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely
	 * within a single render pass.
	 *
	 * - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already
	 * pending execution.
	 *
	 * We have used the last flag because we may already be scheduling the drawing commands for the next frame
	 * while the last frame is not finished yet. The pInheritanceInfo parameter is only relevant for secondary
	 * command buffers. It specifies which state to inherit from the calling primary command buffers.
	 *
	 * If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset
	 * it. It's not possible to append commands to a buffer at a later time.
	 */

	if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS){
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	/**
	 * ## Render Pass
	 */

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[i];

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;

	// Note that the order of clearValues should be identical to the order of your attachments.
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[1].depthStencil = {1.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Begin render pass
	vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind pipeline
	/**
	 * The second parameter specifies if the pipeline object is a graphics or compute pipeline.
	 */
	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[0]);

	// Bind descriptorsstd::vector<VkDescriptorSet> cmdDescriptorSets = { descriptorSets[0].set[i], descriptorSets[1].set[i] };
	std::vector<VkDescriptorSet> cmdDescriptorSets = { descriptorSets[0].set[i], descriptorSets[1].set[i] };
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[0], 0, cmdDescriptorSets.size(), cmdDescriptorSets.data(), 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[1], 0, cmdDescriptorSets.size(), cmdDescriptorSets.data(), 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[2], 0, cmdDescriptorSets.size(), cmdDescriptorSets.data(), 0, nullptr);

	// Draw command
	/**
	 *  vertexCount: Number of vertices (from the vertex buffer)
	 *  instanceCount: Used for instanced rendering, use 1 if you're not doing that.
	 *  firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
	 *  firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex
	 */
	//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

	/**
	 * Just binding an index buffer doesn't change anything yet, we also need to change the drawing command to tell
	 * Vulkan to use the index buffer. Remove the vkCmdDraw line and replace it with vkCmdDrawIndexed:
	 *
	 * indexCount, instanceCount
	 * firstIndex: offset into the index buffer
	 * vertexOffset: offset to add to the indices in the index buffer
	 * firstInstance: offset for instancing
	 */
	//vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	for(int j = 0; j < Storage::renderObjects.size(); ++j){
		RenderComponent *rObj = Storage::renderObjects[j];

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[rObj->pipeline]);

		// Binding the vertex buffer
		/**
		 * The last two parameters specify the array of vertex buffers to bind and the byte offsets to start reading
		 * vertex data from.
		 */
		VkBuffer vertexBuffers[] = { rObj->vertexBuffer->buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

		// Index buffer
		vkCmdBindIndexBuffer(commandBuffers[i], rObj->indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdPushConstants(commandBuffers[i], pipelineLayouts[0], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshTransforms), &rObj->transforms);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(rObj->mesh.indices.size()), 1, 0, 0, 0);
	}

	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[1]);

	if(drawMesh){
		for(Spring *spring : Storage::springs){
			VkBuffer vertexBuffers[] = {spring->vertexBuffer->buffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdPushConstants(commandBuffers[i], pipelineLayouts[1], VK_SHADER_STAGE_VERTEX_BIT, 0,
							   sizeof(MeshTransforms), &RenderComponent::identity);
			vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(spring->vertices.size()), 1, 0, 0);
		}
	}

	// End render pass
	vkCmdEndRenderPass(commandBuffers[i]);

	// Finish recording the command buffer:
	if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS){
		throw std::runtime_error("failed to record command buffer!");
	}
}

/**
 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
 *
 * We'll need one semaphore to signal that an image has been acquired and is ready for rendering, and another one
 * to signal that rendering has finished and presentation can happen. Create two class members to store these
 * semaphore objects:
 *
 * VkSemaphore imageAvailableSemaphore;
 * VkSemaphore renderFinishedSemaphore;
 */
void Vulkan::createSyncObjects(){
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
		if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
		   || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
		   || vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS){

			throw std::runtime_error("failed to create semaphores for a frame!");
		}
	}
}

QueueFamilyIndices Vulkan::findQueueFamilies(VkPhysicalDevice device){
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector <VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for(const auto &queueFamily : queueFamilies){
		if(queueFamily.queueCount > 0){
			if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
				indices.graphicsFamily = i;
			}

			VkBool32
			presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), surface,
												 &presentSupport); // TODO: static_cast ?

			if(presentSupport){
				indices.presentFamily = i;
			}
		}

		if(indices.isComplete()){
			break;
		}

		i++;
	}

	return indices;
}






