#ifndef VULK_VULKAN_H
#define VULK_VULKAN_H

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <set>
#include <fstream>
#include <optional>
#include "BufferAllocation.h"

const int WIDTH = 1500;
const int HEIGHT = 900;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct glMassPoint {
	alignas(16) glm::vec3 force;
	alignas(16) glm::vec3 velocity;
	bool fixed = false;
};

struct glSpring {
	uint32_t first;
	uint32_t second;
	float k;
	float length;
};


struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 normal;
	alignas(8) glm::vec2 texCoord;

	/**
	 * A vertex binding describes at which rate to load data from memory throughout the vertices. It specifies the
	 * number of bytes between data entries and whether to move to the next data entry after each vertex or after
	 * each instance.
	 *
	 * @return
	 */
	static VkVertexInputBindingDescription getBindingDescription() {
		/**
		 * All of our per-vertex data is packed together in one array, so we're only going to have one binding. The
		 * binding parameter specifies the index of the binding in the array of bindings. The stride parameter
		 * specifies the number of bytes from one entry to the next, and the inputRate parameter can have one of the
		 * following values:
		 * - VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		 * - VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
		 */
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	/**
	 * An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data
	 * originating from a binding description. We have two attributes, position and color, so we need two attribute
	 * description structs.
	 *
	 * @return
	 */
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

		/**
		 * The binding parameter tells Vulkan from which binding the per-vertex data comes. The location parameter
		 * references the location directive of the input in the vertex shader. The input in the vertex shader with
		 * location 0 is the position, which has two 32-bit float components.
		 *
		 * The format parameter describes the type of data for the attribute. A bit confusingly, the formats are
		 * specified using the same enumeration as color formats. The following shader types and formats are commonly
		 * used together:
		 * - float: VK_FORMAT_R32_SFLOAT
		 * - vec2: VK_FORMAT_R32G32_SFLOAT
		 * - vec3: VK_FORMAT_R32G32B32_SFLOAT
		 * - vec4: VK_FORMAT_R32G32B32A32_SFLOAT
		 * As you can see, you should use the format where the amount of color channels matches the number of components
		 * in the shader data type. It is allowed to use more channels than the number of components in the shader, but
		 * they will be silently discarded. If the number of channels is lower than the number of components, then the
		 * BGA components will use default values of (0, 0, 1). The color type (SFLOAT, UINT, SINT) and bit width
		 * should also match the type of the shader input. See the following examples:
		 * - ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
		 * - uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
		 * - double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
		 *
		 * The format parameter implicitly defines the byte size of attribute data and the offset parameter specifies
		 * the number of bytes since the start of the per-vertex data to read from. The binding is loading one Vertex
		 * at a time and the position attribute (pos) is at an offset of 0 bytes from the beginning of this struct. This
		 * is automatically calculated using the offsetof macro.
		 */
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
} __attribute__((aligned(16)));

struct PointLight {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 intensity;
};

/*const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
};*/

const std::vector<const char *> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char *> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete(){
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct UBOViewProj {
	glm::mat4 view;
	glm::mat4 proj;
};

struct Light {
	glm::vec3 pos;
	glm::vec3 color;
};

struct UBOLights {
	Light lights[20];
	glm::vec3 eyePos;
};

struct UniformBuffer {
	VkBuffer buffer[MAX_FRAMES_IN_FLIGHT+1];
	VkDeviceMemory memory[MAX_FRAMES_IN_FLIGHT+1];
};

struct DescriptorSet {
	VkDescriptorSetLayout layout;
	VkDescriptorSet set[MAX_FRAMES_IN_FLIGHT+1];
};

class Vulkan {
public:
	struct {
		UBOViewProj viewProj;
		UBOLights lights;
	} uniformBufferObjects;

	bool *running;

	void init();
	void drawFrame();
	void cleanup();

	GLFWwindow *window;
	VkDevice device;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
					  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void recordCommandBuffer(size_t i);

private:

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool commandPool;
	std::vector <VkCommandBuffer> commandBuffers;
	VkCommandBuffer computeCommandBuffer;

	std::vector<UniformBuffer> uniformBuffers;
	std::vector<DescriptorSet> descriptorSets;
	VkDescriptorPool descriptorPool;

	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector <VkImage> swapChainImages;
	std::vector <VkImageView> swapChainImageViews;
	std::vector <VkFramebuffer> swapChainFramebuffers;

	VkImage textureImage;
	VkDeviceMemory textureImageBuffer;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector <VkFence> inFlightFences;
	size_t currentFrame = 0;
	std::vector <VkSemaphore> imageAvailableSemaphores;
	std::vector <VkSemaphore> renderFinishedSemaphores;

	bool framebufferResized = false;

	VkRenderPass renderPass;
	std::vector<VkPipelineLayout> pipelineLayouts;
	std::vector<VkPipeline> graphicsPipelines;

	// Run
	void initMisc();
	void cleanupMisc();
	void initWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height); // from initWindow
	void initVulkan();
	void updateUniformBuffer(uint32_t currentImage); // from drawFrame
	void recreateSwapChain();
		void cleanupSwapChain(); // from recreateSwapChain, clearGarbage

	// initVulkan
	void createInstance();
		bool checkValidationLayerSupport(); // from createInstance
		std::vector<const char *> getRequiredExtensions(); // from createInstance
	void setupDebugMessenger();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
				void *pUserData); // from setupDebugMessenger
	void createSurface();
	void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device); // from pickPhysicalDevice
			bool checkDeviceExtensionSupport(VkPhysicalDevice device); // from isDeviceSuitable
	void createLogicalDevice();
	void createSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); // from createSwapChain
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector <VkSurfaceFormatKHR> &availableFormats); // from createSwapChain
		VkPresentModeKHR chooseSwapPresentMode(const std::vector <VkPresentModeKHR> availablePresentModes); // from createSwapChain
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities); // from createSwapChain
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayouts();
		void createDescriptorSetLayout(VkShaderStageFlags stageFlags, VkDescriptorType descriptorType, uint32_t binding, VkDescriptorSetLayout *layout);
	void createGraphicsPipeline();
		void createTopoPipeline();
		void createLinePipeline();
		void createClothPipeline();
		void createMassPipeline();
		void createSpringPipeline();
		VkShaderModule createShaderModule(const std::vector<char> &code); // from createGraphicsPipeline
	void createFramebuffers();
	void createCommandPool();
	// Images
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	// Depth
	void createDepthResources();
		bool hasStencilComponent(VkFormat format); // from createDepthResources
		VkFormat findDepthFormat(); // from createDepthResources
			VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features); // from findDepthFormat
	void createVertexBuffer();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties); // from createVertexBuffer
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
		void createBufferDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet *pos, UniformBuffer buffer, VkDeviceSize bufferSize);
		void createStorageDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet *pos, VkBuffer buffer, VkDeviceSize bufferSize);
		void createImageDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet *pos, VkImageView imageView, VkSampler imageSampler);
	void createCommandBuffers();
	void createSyncObjects();

	// from createCommandPool, createLogicalDevice, createSwapChain, isDeviceSuitable
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};

#endif //VULK_VULKAN_H
