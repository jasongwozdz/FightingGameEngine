#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "../VkTypes.h"
#include "../../EngineExport.h"

struct PointLightUniformData 
{
	alignas(4)  float constant = 1.0f;
	alignas(4)  float linear = 0.09f;
	alignas(4)  float quadratic = 0.03f;
	alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	alignas(16) glm::vec3 ambient = { 0.0f, 0.0f, 0.0f };
	alignas(16) glm::vec3 diffuse = { 0.0f, 0.0f, 0.0f };
	alignas(16) glm::vec3 specular = { 0.0f, 0.0f, 0.0f };
};

struct DirLightUniformData {
	alignas(16) glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
	alignas(16) glm::vec3 ambient = { 0.0f, 0.0f, 0.0f };
	alignas(16) glm::vec3 diffuse = { 0.0f, 0.0f, 0.0f } ;
	alignas(16) glm::vec3 specular = { 0.0f, 0.0f, 0.0f } ;
};

#define MAX_LIGHTS 64
struct GlobalUniformData
{
	alignas(16) glm::vec3 viewPos;
	DirLightUniformData dirLightData;
	PointLightUniformData pointLightData[MAX_LIGHTS];
	alignas(4) unsigned int numLights;
};

struct MVP
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

struct MVPBoneData : public MVP
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
	int pipelineIndex_;
	UniformDataPtr ubo_;
};

