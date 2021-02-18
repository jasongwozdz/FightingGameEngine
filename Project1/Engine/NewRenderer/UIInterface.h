#pragma once
#include <vulkan/vulkan.h>
#include <Window.h>

class UIInterface
{
public:
	UIInterface(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass);

	~UIInterface();

	void prepareFrame();

	void renderFrame(VkCommandBuffer& curentBuffer);
};
