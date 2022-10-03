#pragma once

#include <iostream>
#include <vector>

#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"

const int MAX_BONES = 64;

struct VulkanBuffer
{
	VkBuffer buffer_;
	VmaAllocation mem_;
};

struct VulkanImage
{
	VkImage image_;
	VmaAllocation mem_;
};

struct TextureResources
{
	//Vulkan Resources
	VulkanImage image_;
	VkImageView view_;
	VkSampler sampler_;
	//OpenGl resources
	unsigned int textureId_;
};

struct PipelineResources
{
	PipelineResources(VkDevice& logicalDevice) :
		logicalDevice_(logicalDevice)
	{};

	~PipelineResources()
	{
		vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
		vkDestroyPipeline(logicalDevice_, pipeline_, nullptr);
	}

	VkDevice& logicalDevice_;
	VkPipeline pipeline_;
	VkPipelineLayout pipelineLayout_;
};

struct PipelineCreateInfo
{
	VkExtent2D windowExtent;
	std::string vertexShader = "";
	std::string fragmentShader = "";
	std::string geometryShader = "";
	std::vector<VkPushConstantRange> pushConstantRanges;
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	bool hasTexture = false;
	bool depthEnabled = true;
	bool hasSkeleton = false;
	bool cullingEnabled = true;
	bool lightingEnabled = true;
	bool isSkybox = false;
	bool isOffscreen = false;
};

struct PipelineCreateInfoHash
{
	size_t operator()(const PipelineCreateInfo& key) const
	{
		size_t hash = std::hash<bool>{}(key.cullingEnabled)^
			std::hash<bool>{}(key.depthEnabled)^
			std::hash<bool>{}(key.hasSkeleton)^
			std::hash<bool>{}(key.hasTexture)^
			std::hash<uint32_t>{}((uint32_t)key.topology)^
			std::hash<uint32_t>{}(key.windowExtent.height)^
			std::hash<uint32_t>{}(key.windowExtent.width)^
			std::hash<std::string>{}(key.vertexShader)^
			std::hash<std::string>{}(key.fragmentShader)^
			std::hash<std::string>{}(key.geometryShader)^
			std::hash<bool>{}(key.cullingEnabled)^
			std::hash<bool>{}(key.lightingEnabled)^
			std::hash<bool>{}(key.isSkybox)^
			std::hash<bool>{}(key.isOffscreen);

		return hash;
	}
};

bool operator==(const PipelineCreateInfo& left, const PipelineCreateInfo& right);



