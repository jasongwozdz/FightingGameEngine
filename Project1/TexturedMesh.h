#pragma once
#include "Mesh.h"
class TexturedMesh :
	public Mesh
{
public:
	TexturedMesh(std::vector<TexturedVertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& m_logicalDevice, std::vector<VkImage>& m_swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath);

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

	virtual void createPipeline();

	virtual void createUniformBuffers();

	virtual void createRenderPass();

	virtual void updateUniformBuffer(uint32_t currentImage);

	virtual void bindToCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFramebuffer>& frameBuffers);
	
	//Textured specific
	void createTextureImages(std::string texturePath);
	
	void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layercount);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	
	void createTextureImageViews();
	
	void createTextureSampler();

};

