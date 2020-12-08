#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "TexturedMesh.h"
#include "BufferOperations.h"
#include "ResourceManager.h"
#include "VulkanErrorCheck.h"
#include "ResourceManager.h"
#include "VulkanImage.h"
#include "Renderer.h"

void TexturedMesh::createTextureImageViews()
{
	m_textureImageView = VulkanImage::createImageView(rm_logicalDevice, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void TexturedMesh::createDescriptorSet()
{
	Renderer& renderer = Renderer::getSingleton();
	std::vector<VkDescriptorSetLayout> layouts(rm_swapChainImages.size(), m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = renderer.getDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(rm_swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(rm_swapChainImages.size());
	if (vkAllocateDescriptorSets(rm_logicalDevice, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(m_ubo);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_textureImageView;
		imageInfo.sampler = m_textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(rm_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void TexturedMesh::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
	m_vertexBufferSize = bufferSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(rm_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(rm_logicalDevice, stagingBufferMemory);

	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	BufferOperations::copyBuffer(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, stagingBuffer, m_vertexBuffer, bufferSize);

	vkDestroyBuffer(rm_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, stagingBufferMemory, nullptr);
}

void TexturedMesh::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_indicies[0]) * m_indicies.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(rm_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_indicies.data(), (size_t)bufferSize);
	vkUnmapMemory(rm_logicalDevice, stagingBufferMemory);

	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	BufferOperations::copyBuffer(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(rm_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, stagingBufferMemory, nullptr);
}

void TexturedMesh::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(m_ubo);

	m_uniformBuffers.resize(rm_swapChainImages.size());
	m_uniformBuffersMemory.resize(rm_swapChainImages.size());

	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
	}
}

void TexturedMesh::createTextureImages(std::string texturePath)
{
	TextureReturnVals vals;
	// TODO : need to decide if loading in all texture data is better done outside of this class eg: loading in models.
	vals = ResourceManager::getSingleton().loadTextureFile(texturePath);
	VkDeviceSize imageSize = vals.textureWidth * vals.textureHeight * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	BufferOperations::createBuffer(rm_logicalDevice, rm_physicalDevice, rm_commandPool, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(rm_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, vals.pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(rm_logicalDevice, stagingBufferMemory);

	//stbi_image_free(vals.pixels);

	VulkanImage::createImage(rm_logicalDevice, rm_physicalDevice, rm_commandPool, vals.textureWidth, vals.textureHeight, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

	VulkanImage::transitionImageLayout(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
	copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(vals.textureWidth), static_cast<uint32_t>(vals.textureHeight));
	VulkanImage::transitionImageLayout(rm_logicalDevice, rm_commandPool, rm_graphicsQueue, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(rm_logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, stagingBufferMemory, nullptr);
}

void TexturedMesh::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BufferOperations::beginSingleTimeCommands(rm_logicalDevice, rm_commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	BufferOperations::endSingleTimeCommands(rm_logicalDevice, rm_graphicsQueue, rm_commandPool, commandBuffer);
}
	
void TexturedMesh::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = 0;
	samplerInfo.mipLodBias = 0;

	if (vkCreateSampler(rm_logicalDevice, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void TexturedMesh::updateUniformBuffer(uint32_t currentImage)
{
	m_ubo.proj = glm::perspective(glm::radians(45.0f), rm_swapChainExtent.width / (float)rm_swapChainExtent.height, 0.1f, 100.0f);
	m_ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(rm_logicalDevice, m_uniformBuffersMemory[currentImage], 0, sizeof(m_ubo), 0, &data);
	memcpy(data, &m_ubo, sizeof(m_ubo));
	vkUnmapMemory(rm_logicalDevice, m_uniformBuffersMemory[currentImage]);
}

TexturedMesh::TexturedMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath, VkDescriptorSetLayout layout):
	Mesh(verticies, indicies, commandBuffers, logicalDevice, swapChainImages, swapChainExtent, physicalDevice, commandPool, graphicsQueue, layout)
{
	createTextureImages(texturePath);
	createTextureImageViews();
	createTextureSampler();

	createUniformBuffers();
	createDescriptorSet();
	createVertexBuffer();
	createIndexBuffer();
}

TexturedMesh::~TexturedMesh()
{
	//texture specific destructors
	vkFreeMemory(rm_logicalDevice, m_textureImageMemory, nullptr);
	vkDestroyImage(rm_logicalDevice, m_textureImage, nullptr);
	vkDestroyImageView(rm_logicalDevice, m_textureImageView, nullptr);
	vkDestroySampler(rm_logicalDevice, m_textureSampler, nullptr);
	//index buffer
	vkDestroyBuffer(rm_logicalDevice, m_indexBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, m_indexBufferMemory, nullptr);
	//vertex buffer
	vkDestroyBuffer(rm_logicalDevice, m_vertexBuffer, nullptr);
	vkFreeMemory(rm_logicalDevice, m_vertexBufferMemory, nullptr);
	//pipeline
	vkDestroyPipeline(rm_logicalDevice, m_pipeline, nullptr);
	vkDestroyPipelineLayout(rm_logicalDevice, m_pipelineLayout, nullptr);
	//uniform buffers
	for (size_t i = 0; i < rm_swapChainImages.size(); i++) {
		vkDestroyBuffer(rm_logicalDevice, m_uniformBuffers[i], nullptr);
		vkFreeMemory(rm_logicalDevice, m_uniformBuffersMemory[i], nullptr);
	}
	//descriptor pool/set
	vkDestroyDescriptorPool(rm_logicalDevice, m_descriptorPool, nullptr);
	std::cout << "deleteing TexturedMesh" << std::endl;
}