#pragma once
#include "Mesh.h"
class TexturedMesh :
	public Mesh
{
public:
	TexturedMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& m_logicalDevice, std::vector<VkImage>& m_swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath);



private:
	/*
	*****************
	Member Variables
	*****************
	*/

	VkImageView textureImageView;

	VkImage textureImage;

	VkDeviceMemory textureImageMemory;

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

	//Textured specific
	void createTextureImages(std::string texturePath);
	
	void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layercount);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	
	void createTextureImageViews();
	
	void createTextureSampler();

};

