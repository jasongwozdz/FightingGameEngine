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

VkShaderModule Mesh::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(rm_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
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

void Mesh::createRenderPass() 
{
	std::cout << "createRenderPass NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};

void Mesh::updateUniformBuffer(uint32_t currentImage)
{

	std::cout << "updateUniformBuffer NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};

void Mesh::bindToCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFramebuffer>& frameBuffers)
{
	std::cout << "bindToCommandBuffers NOT IMPLEMENTED" << std::endl;
	assert(1 == 0);
};