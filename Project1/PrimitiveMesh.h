#pragma once
#include "Mesh.h"

const int WIDTH = 1200;
const int HEIGHT = 900;

class PrimitiveMesh :
	public Mesh
{
public:

	PrimitiveMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);

	~PrimitiveMesh();

	virtual void updateUniformBuffer(uint32_t currentImage);

	void bindToCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFramebuffer>& frameBuffers);


private:

	/*
	*****************
	Member Variables
	*****************	
	*/
		
	/*
	*****************
	Methods
	*****************
	*/
	void createDescriptorSetLayout();

	void createDescriptorPool();

	void createDescriptorSet();

	void createVertexBuffer();
	
	void createIndexBuffer();

	void createUniformBuffers();

	void createRenderPass();
	
};


