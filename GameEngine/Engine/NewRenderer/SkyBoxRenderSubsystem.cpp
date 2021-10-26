#include <iostream>
#include <vector>
#include <filesystem>

#include "SkyBoxRenderSubsystem.h"
#include "../EngineSettings.h"
#include "../Vertex.h"
#include "../ResourceManager.h"


namespace ShaderUtils
{
	std::vector<char> readShaderFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice);
}


SkyBoxRenderSubsystem::SkyBoxRenderSubsystem(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool) :
	RenderSubsystemInterface(logicalDevice, renderPass, allocator, descriptorPool)
{
	textureFaces = new TextureReturnVals[6];
	//createTextureResources(skyboxTexturePath_);
	//createDescriptors();
	//createPipeline();
	//createBuffers();

	//VkDescriptorSetLayoutBinding storageBufferLayoutBinding{};
	//storageBufferLayoutBinding.binding = 0;
	//storageBufferLayoutBinding.descriptorCount = 1; 
	//storageBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	//storageBufferLayoutBinding.pImmutableSamplers = nullptr;
	//storageBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
};

SkyBoxRenderSubsystem::~SkyBoxRenderSubsystem()
{
	delete[] textureFaces;
}

void SkyBoxRenderSubsystem::renderFrame(VkCommandBuffer commandBuffer, uint32_t currentSwapChainIndex)
{
	//std::cout << "Skybox render frame" << std::endl;
}

void SkyBoxRenderSubsystem::createPipeline()
{
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
	skyboxPipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice_, renderPass_, shaders, extent, VK_NULL_HANDLE, {}, true, false, true);

	vkDestroyShaderModule(logicalDevice_, vertexShader, VK_NULL_HANDLE);
	vkDestroyShaderModule(logicalDevice_, fragmentShader, VK_NULL_HANDLE);
}

void SkyBoxRenderSubsystem::createBuffers()
{
	size_t vertexBufferSize = MAX_VERTEX_BUFFER_SIZE * 4 * sizeof(Vertex);

	VkBufferCreateInfo vBufferInfo{};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.pNext = nullptr;
	vBufferInfo.size = vertexBufferSize;
	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &vertexBuffer_.buffer_, &vertexBuffer_.mem_, nullptr);

	size_t indexBufferSize = 2000 * 6 * sizeof(uint32_t);

	VkBufferCreateInfo iBufferInfo{};
	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	iBufferInfo.pNext = nullptr;
	iBufferInfo.size = indexBufferSize;
	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &indexBuffer_.buffer_, &indexBuffer_.mem_, nullptr);
}

bool SkyBoxRenderSubsystem::createTextureResources()
{

	return true;
}

void SkyBoxRenderSubsystem::createDescriptors()
{

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
	for (int i = 0; i < NUMFACES; i++)
	{
		if ( strcmp(filename.c_str(), skyboxFaceFileNames_[i].c_str()) == 0 )
		{
			return i;
		}
	}
}

bool SkyBoxRenderSubsystem::setSkyboxTexture(const std::string& path)//path to folder that includes skybox all sides of sky box
{
	//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	ResourceManager& resourceManager = ResourceManager::getSingleton();

	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		textureFaces[getStringIndex(getFileName(entry.path().string()))] = resourceManager.loadTextureFile(entry.path().string());
	}
	
	createTextureResources();

	return true;
}
