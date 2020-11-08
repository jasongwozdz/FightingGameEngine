#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <set>
#include "VulkanErrorCheck.h"
#include "Vertex.h"
#include "PrimitiveMesh.h"
#include "TexturedMesh.h"
#include "BufferOperations.h"
#include "ResourceManager.h"
#include "GraphicsPipeline.h"
#include "RenderPassComponent.h"
#include "VulkanImage.h"
#include "VulkanErrorCheck.h"
#include "DepthBufferComponent.h"


const int MAX_FRAMES_IN_FLIGHT = 2;

template<> ResourceManager* Singleton<ResourceManager>::msSingleton = 0;

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	"VK_KHR_swapchain"
};


class Renderer
{
public:

	Renderer();
	~Renderer();

	void drawFrame();

	GLFWwindow* getWindow();
	VkDevice getLogicalDevice();

private:
	void init();
	void draw();
	void cleanup();
	void initSingletons();
	void recreateSwapChain();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	void createVulkanInstance();
	static std::vector<char> readFile(const std::string& filename);
	void createCommandPool();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createFramebuffers();
	void createSurface();
	void createLogicalDevice();
	void createPhysicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createSwapChainImageViews();
	void createSyncObjects();
	void initScene();
	void bindMeshesToCommandBuffers(GraphicsPipeline* pipeline, std::vector<Mesh*> meshes);
	void initWindow();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}


	ResourceManager* m_resourceManager;

	GLFWwindow *window;

	VkInstance instance;

	VkCommandPool commandPool;

	bool enableValidationLayers;

	std::vector<VkImageView> swapChainImageViews;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	std::vector<VkCommandBuffer> commandBuffers;

	VkDebugUtilsMessengerEXT debugMessenger;

	VkPhysicalDevice physicalDevice;

	VkDevice logicalDevice;

	VkFormat swapChainImageFormat;;

	VkExtent2D swapChainExtent;

	VkSwapchainKHR swapChain;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkPhysicalDeviceFeatures supportedFeatures;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	QueueFamilyIndices queueFamilyIndices;

	VkSurfaceKHR surface;

	std::vector<VkImage> swapChainImages;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;;

	//maps for each mesh
	std::map<GraphicsPipeline*, std::vector<Mesh*>> pipelineMap;

	size_t currentFrame = 0;

	bool framebufferResized = false;

};

