#include "Mesh.h"
#include <fstream>
#include <iostream>

struct RenderUploadInfo
{
	std::vector<Vertex>* vertices;
	std::vector<uint32_t>* indices;
	bool animated;
	int topology; //topology 0 = triangle, 1 = line
	bool textured; //textured
};

Mesh::Mesh(std::vector<Vertex>& verticies, std::vector<uint32_t>& indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkDescriptorSetLayout layout) :
	m_vertices(verticies),
	m_indicies(indicies),
	rm_commandBuffers(commandBuffers),
	rm_logicalDevice(logicalDevice),
	rm_swapChainImages(swapChainImages),
	rm_swapChainExtent(swapChainExtent),
	rm_physicalDevice(physicalDevice),
	rm_commandPool(commandPool),
	rm_graphicsQueue(graphicsQueue),
	m_descriptorSetLayout(layout)
{};

Mesh::Mesh(const Mesh&& mesh) :
	m_vertices(mesh.m_vertices),
	m_indicies(mesh.m_indicies),
	rm_commandBuffers(mesh.rm_commandBuffers),
	rm_logicalDevice(mesh.rm_logicalDevice),
	rm_swapChainImages(mesh.rm_swapChainImages),
	rm_swapChainExtent(mesh.rm_swapChainExtent),
	rm_physicalDevice(mesh.rm_physicalDevice),
	rm_commandPool(mesh.rm_commandPool),
	rm_graphicsQueue(mesh.rm_graphicsQueue),
	m_descriptorSetLayout(mesh.m_descriptorSetLayout)
{};

Mesh::~Mesh() {};

void Mesh::draw()
{
	RenderUploadInfo info;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	bool animated;
	int topology; //topology 0 = triangle, 1 = line
	bool textured; //textured
}

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


