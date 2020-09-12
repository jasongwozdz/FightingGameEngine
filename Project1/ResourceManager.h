#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <vector>

struct Vertex;

class ResourceManager
{

	std::map<std::string, uintptr_t> m_resourceRegistry;

	std::vector<Vertex> loadObjFile(std::string& filePath);

	void createTextureImage(std::string texturePath);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
};

