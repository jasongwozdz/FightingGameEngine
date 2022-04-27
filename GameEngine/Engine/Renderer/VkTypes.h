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
	VulkanImage image_;
	VkImageView view_;
	VkSampler sampler_;
};

struct PipelineResources
{
	PipelineResources(VkDevice& logicalDevice) :
		logicalDevice_(logicalDevice)
	{};

	~PipelineResources()
	{
		std::cout << "destroy PipelineResources" << std::endl;
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
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	std::string vertexShader = "./shaders/texturedMeshVert.spv";
	std::string fragmentShader = "./shaders/texturedMeshFrag.spv";
	std::vector<VkPushConstantRange> pushConstantRanges;
	bool hasTexture = false;
	bool depthEnabled = true;
	bool hasSkeleton = false;
	bool cullingEnabled = true;
	bool lightingEnabled = false;
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
			std::hash<std::string>{}(key.fragmentShader);

		return hash;
	}
};

bool operator==(const PipelineCreateInfo& left, const PipelineCreateInfo& right);



