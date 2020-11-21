#pragma once
#include "Mesh.h"
#include "ResourceManager.h"

class DepthBuffer;

class TexturedMesh :
	public Mesh
{
public:
	TexturedMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& m_logicalDevice, std::vector<VkImage>& m_swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath);

	~TexturedMesh();

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
	/*
	*****************
	Methods
	*****************
	*/
	//required methods
	virtual void createDescriptorSetLayout();

	virtual void createDescriptorPool();

	virtual void createDescriptorSet();

	virtual void createVertexBuffer();

	virtual void createIndexBuffer();

	virtual void createUniformBuffers();

	virtual void updateUniformBuffer(uint32_t currentImage);
	
	//Textured specific
	void createTextureImages(std::string texturePath);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	void createTextureImageViews();
	
	void createTextureSampler();

};

