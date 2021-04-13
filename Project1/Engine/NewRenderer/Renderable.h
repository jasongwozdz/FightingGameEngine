#pragma once
#include <vector>
#include <string>
#include "../Vertex.h"
#include "VkTypes.h"
#include <iostream>
#include <memory>

#define MAX_BONES 64

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
	NUM_PIPELINE_TYPES = 4
};

struct TextureResources
{
	VulkanImage image_;
	VkImageView view_;
	VkSampler sampler_;
};

class Renderable
{
public:
	Renderable();

	Renderable(std::vector<Vertex> vertex, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName);

	Renderable(Renderable&& other);

	Renderable(const Renderable& other);

	Renderable& operator=(Renderable&& other)
	{
		std::cout << "Move assignement operator" << std::endl;
		depthEnabled_ = other.depthEnabled_;
		draw_ = other.draw_;
		uploaded_ = other.uploaded_;
		delete_ = other.delete_;
		isLine_ = other.isLine_;
		ubo_ = new Ubo;
		memcpy(ubo_, other.ubo_, sizeof(Ubo));
		vertexBuffer_ = other.vertexBuffer_;
		other.vertexBuffer_ = VK_NULL_HANDLE;
		vertexMem_ = other.vertexMem_;
		other.vertexMem_ = nullptr;
		indexBuffer_ = other.indexBuffer_;
		other.indexBuffer_ = VK_NULL_HANDLE;
		indexMem_ = other.indexMem_;
		other.indexMem_ = nullptr;
		for (int i = 0; i < other.uniformBuffer_.size(); i++)
		{
			uniformBuffer_[i] = other.uniformBuffer_[i];
			other.uniformBuffer_[i] = VK_NULL_HANDLE;
			uniformMem_[i] = other.uniformMem_[i];
			other.uniformMem_[i] = nullptr;
		}
		renderableObjectId_ = (other.renderableObjectId_);
		descriptorLayout_ = (other.descriptorLayout_);
		descriptorSets_ = (other.descriptorSets_);
		allocator_ = (other.allocator_);
		logicalDevice_ = other.logicalDevice_;
		textureResources_ = new TextureResources;
		memcpy(textureResources_, other.textureResources_, sizeof(TextureResources));
		return *this;
	}

	Renderable& operator=(Renderable const& other)
	{
		std::cout << "JASON: Assigned" << std::endl;
		depthEnabled_ = other.depthEnabled_;
		draw_ = other.draw_;
		uploaded_ = other.uploaded_;
		delete_ = other.delete_;
		isLine_ = other.isLine_;
		ubo_ = new Ubo;
		memcpy(ubo_, other.ubo_, sizeof(Ubo));
		vertexBuffer_ = other.vertexBuffer_;
		vertexMem_ = other.vertexMem_;
		indexBuffer_ = other.indexBuffer_;
		indexMem_ = other.indexMem_;
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
		textureResources_ = new TextureResources;
		memcpy(textureResources_, other.textureResources_, sizeof(TextureResources));
		return *this;
	}

	~Renderable();
	
	Ubo* ubo_;

	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	std::string entityName_;
	
	Ubo& ubo();
	bool depthEnabled_;
	bool draw_;
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

	PipelineTypes pipelineType_ = PipelineTypes::BASIC_PIPELINE;

	uint32_t renderableObjectId_;

	VkDescriptorSetLayout descriptorLayout_;
	std::vector<VkDescriptorSet> descriptorSets_;

	TextureResources* textureResources_;

	void deleteResources(VmaAllocator& allocator, VkDevice& logicalDevice);
};
