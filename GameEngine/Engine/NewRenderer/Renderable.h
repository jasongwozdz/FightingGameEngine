#pragma once
#include <vector>
#include <string>
#include "../Vertex.h"
#include "VkTypes.h"
#include <iostream>
#include <memory>

#define MAX_BONES 64

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

struct Ubo 
{
	alignas(16)glm::mat4 model;
	alignas(16)glm::mat4 view;
	alignas(16)glm::mat4 proj;
	alignas(16)glm::mat4 bones[MAX_BONES];
};

enum PipelineTypes 
{
	BASIC_PIPELINE,
	LINE_PIPELINE,
	DEBUG_PIPELINE,
	ANIMATION_PIPELINE,
	GUI_PIPELINE,
	NUM_PIPELINE_TYPES = 5
};

struct TextureResources
{
	VulkanImage image_;
	VkImageView view_;
	VkSampler sampler_;
};

class ENGINE_API Renderable
{
public:
	Renderable(std::vector<Vertex> vertex, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName);

	Renderable(Renderable&& other);

	Renderable(const Renderable& other);

	~Renderable();

	//repopulates vertex and index buffers
	void setVertAndIndicies(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	void deleteResources(VmaAllocator& allocator, VkDevice& logicalDevice);

	Renderable& operator=(Renderable&& other)
	{
		depthEnabled_ = other.depthEnabled_;
		uploaded_ = other.uploaded_;
		delete_ = other.delete_;
		isLine_ = other.isLine_;
		ubo_ = other.ubo_;
		vertexBuffer_ = other.vertexBuffer_;
		other.vertexBuffer_ = VK_NULL_HANDLE;
		vertexMem_ = other.vertexMem_;
		other.vertexMem_ = nullptr;
		indexBuffer_ = other.indexBuffer_;
		other.indexBuffer_ = VK_NULL_HANDLE;
		indexMem_ = other.indexMem_;
		other.indexMem_ = nullptr;
		isLine_ = other.isLine_;
		for (int i = 0; i < other.uniformBuffer_.size(); i++)
		{
			uniformBuffer_[i] = other.uniformBuffer_[i];
			other.uniformBuffer_[i] = VK_NULL_HANDLE;
			uniformMem_[i] = other.uniformMem_[i];
			other.uniformMem_[i] = nullptr;
		}
		renderableObjectId_ = (other.renderableObjectId_);
		descriptorLayout_ = (other.descriptorLayout_);
		other.descriptorLayout_ = VK_NULL_HANDLE;
		descriptorSets_ = (other.descriptorSets_);
		for (int i = 0; i < other.descriptorSets_.size(); i++)
		{
			other.descriptorSets_[i] = VK_NULL_HANDLE;
		}
		allocator_ = (other.allocator_);
		logicalDevice_ = other.logicalDevice_;
		textureResources_ = other.textureResources_;
		return *this;
	}

	Renderable& operator=(Renderable const& other)
	{
		depthEnabled_ = other.depthEnabled_;
		uploaded_ = other.uploaded_;
		delete_ = other.delete_;
		isLine_ = other.isLine_;
		ubo_ = other.ubo_;
		vertexBuffer_ = other.vertexBuffer_;
		vertexMem_ = other.vertexMem_;
		indexBuffer_ = other.indexBuffer_;
		indexMem_ = other.indexMem_;
		isLine_ = other.isLine_;
		for (int i = 0; i < other.uniformBuffer_.size(); i++)
		{
			uniformBuffer_[i] = other.uniformBuffer_[i];
			uniformMem_[i] = other.uniformMem_[i];
		}
		renderableObjectId_ = (other.renderableObjectId_);
		descriptorLayout_ = (other.descriptorLayout_);
		descriptorSets_ = (other.descriptorSets_);
		allocator_ = (other.allocator_);
		logicalDevice_ = other.logicalDevice_;
		textureResources_ = other.textureResources_;
		return *this;
	}

public:
	Ubo ubo_;

	PipelineTypes pipelineType_ = PipelineTypes::BASIC_PIPELINE;

	uint32_t renderableObjectId_;

	TextureResources textureResources_;

	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	std::string entityName_;
	
	bool depthEnabled_;
	bool uploaded_;
	bool delete_;
	bool isLine_ = false;
	bool render_ = true;

	VkDevice logicalDevice_;
	VmaAllocator allocator_;

	VkBuffer vertexBuffer_;
	VmaAllocation vertexMem_;

	VkBuffer indexBuffer_;
	VmaAllocation indexMem_;

	std::vector<VkBuffer> uniformBuffer_;
	std::vector<VmaAllocation> uniformMem_;

	VkDescriptorSetLayout descriptorLayout_;
	std::vector<VkDescriptorSet> descriptorSets_;

};
