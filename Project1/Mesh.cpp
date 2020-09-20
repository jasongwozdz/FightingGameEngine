#include "Mesh.h"
#include <fstream>
#include <iostream>

Mesh::Mesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue) :
	m_vertices(verticies),
	m_indicies(indicies),
	rm_commandBuffers(commandBuffers),
	rm_logicalDevice(logicalDevice),
	rm_swapChainImages(swapChainImages),
	rm_swapChainExtent(swapChainExtent),
	rm_physicalDevice(physicalDevice),
	rm_commandPool(commandPool),
	rm_graphicsQueue(graphicsQueue)
{};

Mesh::Mesh(std::vector<TexturedVertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue) :
	m_texturedVertices(verticies),
	m_indicies(indicies),
	rm_commandBuffers(commandBuffers),
	rm_logicalDevice(logicalDevice),
	rm_swapChainImages(swapChainImages),
	rm_swapChainExtent(swapChainExtent),
	rm_physicalDevice(physicalDevice),
	rm_commandPool(commandPool),
	rm_graphicsQueue(graphicsQueue)
{};

Mesh::~Mesh() {};

void Mesh::setWorldMatrix(glm::mat4 modelToWorld)
{
	m_ubo.model = modelToWorld;
}

void Mesh::setViewMatrix(glm::mat4 worldToView)
{
	m_ubo.view = worldToView;
}

void Mesh::setProjectionMatrix(glm::mat4 projection) 
{
	m_ubo.proj = projection;
}
//virtual
void Mesh::createDescriptorSetLayout() 
{
	std::cout << "createDescriptorSetLayout NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};

void Mesh::createUniformBuffers()
{
	std::cout << "createUniformBuffers NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};

void Mesh::updateUniformBuffer(uint32_t currentImage)
{
	std::cout << "updateUniformBuffer NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};


