#define STB_IMAGE_IMPLEMENTATION
#include "TexturedMesh.h"
#include "BufferOperations.h"




VkImage createVulkanImage() {

}

void TexturedMesh::createTextureImageViews() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

TexturedMesh::TexturedMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, std::vector<VkCommandBuffer>& commandBuffers, VkDevice& logicalDevice, std::vector<VkImage>& swapChainImages, VkExtent2D& swapChainExtent, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue, std::string& texturePath):
	Mesh(verticies, indicies, commandBuffers, logicalDevice, swapChainImages, swapChainExtent, physicalDevice, commandPool, graphicsQueue)
{
	createTextureImages(texturePath);
	createTextureImageViews();
	createTextureSampler();

	createUniformBuffers();
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSet();
	createRenderPass();
	createPipeline();
	createVertexBuffer();
	createIndexBuffer();
}