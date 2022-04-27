#include <iostream>
#include <vector>
#include <filesystem>

#include "SkyBoxRenderSubsystem.h"
#include "../EngineSettings.h"
#include "../Vertex.h"
#include "../ResourceManager.h"
#include "VkRenderer.h"
#include "../Scene/Scene.h"
#include "../Scene/Components/Camera.h"

#define NUM_FACES 6

namespace ShaderUtils
{
	std::vector<char> readShaderFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice);
}


SkyBoxRenderSubsystem::SkyBoxRenderSubsystem(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool, VkRenderer& renderer) :
	scene_(Scene::getSingletonPtr()),
	RenderSubsystemInterface(logicalDevice, renderPass, allocator, descriptorPool, renderer)
{};

SkyBoxRenderSubsystem::~SkyBoxRenderSubsystem()
{
}

void SkyBoxRenderSubsystem::renderFrame(VkCommandBuffer commandBuffer, uint32_t currentSwapChainIndex)
{
	if (!skyboxSet_)
	{
		return;
	}
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline_->pipeline_);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_.buffer_, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer_.buffer_, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline_->pipelineLayout_, 0, 1, &skyboxDescriptorSet_, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(numSkyboxModelIndicies_), 1, 0, 0, 0);

	updateUniformBuffer();
}

void SkyBoxRenderSubsystem::onResize(VkCommandBuffer commandBuffer)
{

}

void SkyBoxRenderSubsystem::updateUniformBuffer()
{
	Camera& camera = scene_->getCurrentCamera();
	Transform& transform = camera.entity_->getComponent<Transform>();
	glm::mat4 view, projection;
	scene_->calculateViewProjection(view, projection);
	skyboxBufferObject_.model = glm::translate(glm::mat4(1.0f), transform.position_);
	skyboxBufferObject_.model = glm::scale(skyboxBufferObject_.model, { 5.0f, 5.0f, 5.0f });
	skyboxBufferObject_.proj = projection;
	skyboxBufferObject_.view = view;
	void *data;
	size_t y = sizeof(Ubo);
	vmaMapMemory(allocator_, uniformBuffer_.mem_, &data);
	memcpy(data, &skyboxBufferObject_, sizeof(Ubo));
	vmaUnmapMemory(allocator_, uniformBuffer_.mem_);
}

void SkyBoxRenderSubsystem::createPipeline()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	//VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	//samplerLayoutBinding.binding = 1;
	//samplerLayoutBinding.descriptorCount = 1;
	//samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//samplerLayoutBinding.pImmutableSamplers = nullptr;
	//samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &skyboxDescriptorSetLayout_);

	std::vector<char> vertexShaderCode = ShaderUtils::readShaderFile(VERT_SHADER_);
	std::vector<char> fragmentShaderCode = ShaderUtils::readShaderFile(FRAG_SHADER_);
	VkShaderModule vertexShader = ShaderUtils::createShaderModule(vertexShaderCode, logicalDevice_);
	VkShaderModule fragmentShader = ShaderUtils::createShaderModule(fragmentShaderCode, logicalDevice_);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShader;
	fragShaderStageInfo.pName = "main";
	
	std::vector<VkPipelineShaderStageCreateInfo> shaders = { vertShaderStageInfo, fragShaderStageInfo };

	VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };
	skyboxPipeline_ = PipelineBuilder::createPipeline<NonAnimVertex>(logicalDevice_, renderPass_, shaders, extent, &skyboxDescriptorSetLayout_, {}, false, false, false);

	vkDestroyShaderModule(logicalDevice_, vertexShader, VK_NULL_HANDLE);
	vkDestroyShaderModule(logicalDevice_, fragmentShader, VK_NULL_HANDLE);
}

void SkyBoxRenderSubsystem::createUniformBuffers()
{
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = sizeof(skyboxBufferObject_);
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &uniformBuffer_.buffer_, &uniformBuffer_.mem_, nullptr);
}

bool SkyBoxRenderSubsystem::createTextureResources(int baseHeight, int baseWidth, int numChannels, std::vector<unsigned char>& totalPixels)
{
	VkExtent3D extent = {baseHeight, baseWidth, 1};
	VkImageCreateInfo textureInfo = vkinit::image_create_info(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	
	skyboxTexture_ = renderer_.createTextureResources(baseWidth, baseHeight, numChannels, baseWidth*baseHeight * 4, totalPixels, textureInfo);//create image and image view

	VkSamplerCreateInfo samplerCreateInfo = vkinit::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	vkCreateSampler(logicalDevice_, &samplerCreateInfo, nullptr, &skyboxTexture_.sampler_);

	return true;
}

void SkyBoxRenderSubsystem::createDescriptors()
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool_;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &skyboxDescriptorSetLayout_;

	vkAllocateDescriptorSets(logicalDevice_, &allocInfo, &skyboxDescriptorSet_);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer_.buffer_;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(skyboxBufferObject_);

	//VkDescriptorImageInfo imageInfo{};
	//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//imageInfo.imageView = skyboxTexture_.view_;
	//imageInfo.sampler = skyboxTexture_.sampler_;

	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = skyboxDescriptorSet_;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	//descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//descriptorWrites[1].dstSet = skyboxDescriptorSet_;
	//descriptorWrites[1].dstBinding = 1;
	//descriptorWrites[1].dstArrayElement = 0;
	//descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//descriptorWrites[1].descriptorCount = 1;
	//descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::string getFileName(const std::string& path)
{
	auto found = path.rfind('jpg');
	if (found != std::string::npos)
	{
		auto end = path.rfind('\\');
		if (end != std::string::npos)
		{
			return path.substr(end + 1, found - end - 4);
		}
	}
}

int SkyBoxRenderSubsystem::getStringIndex(const std::string& filename)
{
	for (int i = 0; i < NUM_FACES; i++)
	{
		if ( strcmp(filename.c_str(), skyboxFaceFileNames_[i].c_str()) == 0 )
		{
			return i;
		}
	}
}

void SkyBoxRenderSubsystem::loadCube(ModelReturnVals* modelVals)
{
	size_t vertexBufferSize = modelVals->vertices.size() * sizeof(NonAnimVertex);

	VkBufferCreateInfo vBufferInfo{};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.pNext = nullptr;
	vBufferInfo.size = vertexBufferSize;
	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	VK_CHECK(vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &vertexBuffer_.buffer_, &vertexBuffer_.mem_, nullptr));

	void *vertexData;
	vmaMapMemory(allocator_, vertexBuffer_.mem_, &vertexData);
	memcpy(vertexData, modelVals->vertices.data(), vertexBufferSize);
	vmaUnmapMemory(allocator_, vertexBuffer_.mem_);

	//======= Indicies ======
	size_t indexBufferSize = modelVals->indices.size() * sizeof(uint32_t);

	VkBufferCreateInfo iBufferInfo{};
	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	iBufferInfo.pNext = nullptr;
	iBufferInfo.size = indexBufferSize;
	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	VK_CHECK(vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &indexBuffer_.buffer_, &indexBuffer_.mem_, nullptr));

	void *indexData;
	vmaMapMemory(allocator_, indexBuffer_.mem_, &indexData);
	memcpy(indexData, modelVals->indices.data(), indexBufferSize);
	vmaUnmapMemory(allocator_, indexBuffer_.mem_);
}

bool SkyBoxRenderSubsystem::setSkyboxTexture(const std::string& path)
{
	skyboxSet_ = true;
	ResourceManager& resourceManager = ResourceManager::getSingleton();

	int count = 0;
	std::vector<unsigned char> totalPixels;
	int baseWidth = 0;
	int baseHeight = 0;
	int numChannels = 0;

	for (int i = 0; i < 6; i++)
	{
		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string fullPath = entry.path().string();
			size_t cutoff = fullPath.find_last_of("\\") + 1;
			std::string fileName = fullPath.substr(cutoff);
			cutoff = fileName.find_last_of(".");
			fileName = fileName.substr(0, cutoff);
			if (fileName == skyboxFaceFileNames_[i])
			{
				TextureReturnVals vals = resourceManager.loadTextureFile(entry.path().string());
				baseWidth = vals.textureWidth;
				baseHeight = vals.textureHeight;
				totalPixels.insert(totalPixels.end(),vals.pixels.begin(), vals.pixels.end());
			}
		}

	}
	if (totalPixels.size() <= 0)
	{
		std::cout << "ERROR couldn't load skybox" << std::endl;
		assert(totalPixels.size() <= 0);
	}
	
	//createTextureResources(baseHeight, baseWidth, numChannels, totalPixels);
	ModelReturnVals modelVals = resourceManager.loadObjFile(CUBE_MODEL_PATH_);
	numSkyboxModelIndicies_ = modelVals.indices.size();
	loadCube(&modelVals);
	createUniformBuffers();
	createPipeline();
	createDescriptors();

	return true;
}

