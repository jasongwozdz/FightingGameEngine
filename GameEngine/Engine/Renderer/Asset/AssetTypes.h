#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "../VkTypes.h"
#include "../../EngineExport.h"

struct ENGINE_API MVP
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

struct ENGINE_API MVPBoneData : public MVP
{
	alignas(16) glm::mat4 bones_[MAX_BONES];	
};

typedef MVP* UniformDataPtr;

struct ENGINE_API DynamicAssetData 
{
	DynamicAssetData()
	{};

	~DynamicAssetData()
	{
		delete ubo_;
	}

	DynamicAssetData(DynamicAssetData&& other)
	{
		uniformData_ = other.uniformData_;
		descriptorLayout_ = other.descriptorLayout_;
		descriptorSets_ = other.descriptorSets_;
		pipelineIndex_ = other.pipelineIndex_;
		ubo_ = other.ubo_;
		other.ubo_ = nullptr;
	}

	DynamicAssetData& operator=(DynamicAssetData&& other)
	{
		uniformData_ = other.uniformData_;
		descriptorLayout_ = other.descriptorLayout_;
		descriptorSets_ = other.descriptorSets_;
		pipelineIndex_ = other.pipelineIndex_;
		ubo_ = other.ubo_;
		other.ubo_ = nullptr;
		return *this;
	}

	std::vector<VulkanBuffer> uniformData_;
	VkDescriptorSetLayout descriptorLayout_ = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets_;
	std::vector<VkDescriptorSet> offscreenDescriptorSet_;
	VkDescriptorSetLayout offscreenDescriptorLayout_;
	UniformDataPtr ubo_;
	int pipelineIndex_;
};

