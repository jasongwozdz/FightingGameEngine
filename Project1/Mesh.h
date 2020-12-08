#pragma once
#include <array>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include "Vertex.h"

//Forward declerations
class mesh_IndexBufferComp;
class mesh_NormalComp;
class mesh_TextureComp;
class mesh_ShaderComp;

struct UBO{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;

	UBO() {model = glm::mat4(1.0f);  view = glm::lookAt(glm::vec3(1.0f, 3.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); };

	UBO(glm::mat4 m, glm::mat4 v, glm::mat4 p) : model(m), view(v), proj(p) {};
};

class Mesh
{
public:
	UBO m_ubo;
	Mesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkDescriptorSetLayout layout);

	virtual ~Mesh();

	virtual void updateUniformBuffer(uint32_t currentImage);

	VkBuffer m_vertexBuffer;

	VkPipeline m_pipeline;
	
	VkPipelineLayout m_pipelineLayout;
	
	VkBuffer m_indexBuffer;

	std::vector<VkDescriptorSet> m_descriptorSets;

	VkDescriptorSetLayout m_descriptorSetLayout;

	std::vector<Vertex> m_vertices;

	std::vector<uint32_t> m_indicies;

	void setWorldMatrix(glm::mat4 modelToWorld);

	void setViewMatrix(glm::mat4 worldToView);

	void setProjectionMatrix(glm::mat4 projection);
		
protected:

	/*
	*****************
	Member Variables
	*****************
	*/

	//components
	static mesh_ShaderComp  *m_shaderComp;
	mesh_IndexBufferComp *m_indexBufferComp;
	mesh_NormalComp	*m_normalComp;
	mesh_TextureComp *m_textureComp;
	//end componenets

	int m_uniformBufferIndex;

	int m_uniformBufferSize;

	int m_indexCount;

	VkDeviceMemory m_vertexBufferMemory;

	VkDeviceSize m_vertexBufferSize;

	VkDeviceMemory m_indexBufferMemory;

	std::vector<VkBuffer> m_uniformBuffers;

	std::vector<VkDeviceMemory> m_uniformBuffersMemory;

	VkDescriptorPool m_descriptorPool;

	/*
	*****************
	References
	*****************
	*/

	VkDevice& rm_logicalDevice;
	
	std::vector<VkCommandBuffer>& rm_commandBuffers;

	std::vector<VkImage>& rm_swapChainImages;

	VkExtent2D& rm_swapChainExtent;

	VkPhysicalDevice& rm_physicalDevice;
	
	VkCommandPool& rm_commandPool;
	
	VkQueue& rm_graphicsQueue;

	/*
	*****************
	Methods
	*****************
	*/

	virtual void createDescriptorSetLayout();

	virtual void createUniformBuffers();







};
