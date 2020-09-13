#pragma once
#include <vulkan/vulkan.h>

/*
static class used for various operations involving Vulkan buffers
*/

class BufferOperations
{
public:

	static void createBuffer(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	static void copyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	static VkCommandBuffer beginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool);

	static void endSingleTimeCommands(VkDevice logicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	static uint32_t findMemoryType(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

