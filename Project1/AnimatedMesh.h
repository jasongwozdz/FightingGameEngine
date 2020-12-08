#pragma once
#include "Mesh.h"

#define MAX_BONES 64

struct Anim_UBO{
 	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::mat4 bones[MAX_BONES];
	Anim_UBO() {};

	Anim_UBO(UBO u) : model(u.model), view(u.view), proj(u.proj) {};
};

class AnimatedMesh :
	public Mesh
{
public:
	Anim_UBO animUbo;

	AnimatedMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& m_logicalDevice, std::vector<VkImage>& m_swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath, VkDescriptorSetLayout layout);

	~AnimatedMesh();

	virtual void createDescriptorSet();

	virtual void createVertexBuffer();

	virtual void createIndexBuffer();

	virtual void createUniformBuffers();

	virtual void updateUniformBuffer(uint32_t currentImage);

private:
	/*
	*****************
	Member Variables
	*****************
	*/
	VkImageView m_textureImageView;

	VkImage m_textureImage;

	VkDeviceMemory m_textureImageMemory;
	
	VkSampler m_textureSampler;

	void createTextureImages(std::string texturePath);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	void createTextureImageViews();
	
	void createTextureSampler();
};

