#pragma once
#include <vulkan/vulkan.h>

/*
static class used for various operations involving Vulkan buffers
*/

class BufferOperations
{
public:
	static VkDevice& rm_logicalDevice;

	static VkPhysicalDevice& rm_physicalDevice;

	static VkCommandPool& rm_commandPool;

	static VkQueue& rm_graphicsQueue;

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	static void copyBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

	static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};

