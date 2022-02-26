#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "../VkTypes.h"

struct MVP
{
	alignas(16) glm::mat4 mvpMatrix_;
};

typedef MVP* UniformDataPtr;

struct MVPBoneData : public MVP
{
	alignas(16) glm::mat4 bones_[MAX_BONES];
};

struct DynamicAssetData 
{
	std::vector<VulkanBuffer> uniformData_;
	VkDescriptorSetLayout descriptorLayout_;
	std::vector<VkDescriptorSet> descriptorSets_;
	UniformDataPtr ubo_;
	PipelineResources* pipeline_;
};

