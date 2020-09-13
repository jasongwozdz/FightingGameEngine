#pragma once
#include <array>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Vertex.h"



//Forward declerations
class mesh_IndexBufferComp;
class mesh_NormalComp;
class mesh_TextureComp;
class mesh_ShaderComp;

class Mesh
{
public:
	struct {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	}ubo;

	Mesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);

	Mesh(std::vector<TexturedVertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);

	virtual ~Mesh();

	virtual void updateUniformBuffer(uint32_t currentImage);

	virtual void bindToCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkFramebuffer>& frameBuffers);

	VkRenderPass m_renderPass;

	static std::vector<char> readShaderFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	};

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
	
	std::vector<Vertex> m_vertices;

	std::vector<TexturedVertex> m_texturedVertices;

	std::vector<uint32_t> m_indicies;

	VkBuffer m_vertexBuffer;

	VkDeviceMemory m_vertexBufferMemory;

	VkDeviceSize m_vertexBufferSize;

	VkBuffer m_indexBuffer;

	VkDeviceMemory m_indexBufferMemory;

	std::vector<VkBuffer> m_uniformBuffers;

	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	

	VkDescriptorPool m_descriptorPool;

	VkDescriptorSetLayout m_descriptorSetLayout;

	std::vector<VkDescriptorSet> m_descriptorSets;

	VkPipelineLayout m_pipelineLayout;

	VkPipeline m_pipeline;


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
	VkShaderModule createShaderModule(const std::vector<char>& code);

	virtual void createDescriptorSetLayout();

	virtual void createUniformBuffers();

	virtual void createRenderPass();






};
