#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include "VulkanErrorCheck.h"


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


class VulkanBase
{
public:

	static VulkanBase& getVulkanBaseInstance();

	static VulkanBase& getVulkanBaseInstance(VkInstance vkinstance, GLFWwindow *window);

	//default destructor 
	~VulkanBase();

	VkDebugUtilsMessengerEXT debugMessenger;

	VkPhysicalDevice physicalDevice;

	VkDevice logicalDevice;

	VkFormat swapChainImageFormat;

	VkExtent2D swapChainExtent;

	VkSwapchainKHR swapChain;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkPhysicalDeviceFeatures supportedFeatures;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	QueueFamilyIndices queueFamilyIndices;

	static void loadMesh(std::string);


private:

	static VulkanBase vulkanBaseInstance;

	//default Constructer: set enableValidationLayers to true
	VulkanBase(VkInstance vkinstance, GLFWwindow *window);

	VkInstance instance;

	std::vector<const char*> getRequiredExtensions();

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	
	GLFWwindow* window;

	VkSurfaceKHR surface;

	std::vector<VkImageView> swapChainImageViews;

	std::vector<VkImage> swapChainImages;
	
	VkSampleCountFlagBits getMaxUsableSampleCount(); 

	//must be set
	bool enableValidationLayers;

	VkDebugUtilsMessengerEXT setupDebugMessenger();

	bool checkValidationLayerSupport();

	void createVulkanInstance();

	void createLogicalDevice();

	void createPhysicalDevice();

	void createSwapChain();

	void createImageViews();

	void createSurface();


	bool isDeviceSuitable(VkPhysicalDevice device);

	void findQueueFamilies(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

};

