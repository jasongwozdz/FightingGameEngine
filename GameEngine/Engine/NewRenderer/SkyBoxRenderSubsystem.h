#pragma once
#include <vulkan/vulkan.h>
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"

#include "RenderSubsystemInterface.h"
#include "VkTypes.h"
#include "PipelineBuilder.h"

#define MAX_VERTEX_BUFFER_SIZE 1000

enum SkyboxFaces
{
	FRONT = 0,
	BOTTOM,
	LEFT,
	RIGHT,
	BACK,
	TOP,
	NUMFACES
};

struct TextureReturnVals;

class SkyBoxRenderSubsystem : public RenderSubsystemInterface
{
public:
	SkyBoxRenderSubsystem(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool);

	~SkyBoxRenderSubsystem();

	void renderFrame(VkCommandBuffer commandBuffer, uint32_t currentSwapChainIndex) override;

	bool setSkyboxTexture(const std::string& path);//true if file exists and skybox loading is succesful
private:
	bool createTextureResources();
	void createDescriptors();
	void createPipeline();
	void createBuffers();
	void createTextureResource(TextureReturnVals* textureVals);
	int getStringIndex(const std::string& filename);

private:
	std::string skyboxFaceFileNames_[6] =
	{
		"front",
		"bottom",
		"left",
		"right",
		"back",
		"top"
	};

	TextureResources skyboxTexture_;
	VulkanBuffer vertexBuffer_;
	VulkanBuffer indexBuffer_;
	PipelineBuilder::PipelineResources* skyboxPipeline_;
	const std::string VERT_SHADER_ = "";
	const std::string FRAG_SHADER_ = "";
	std::string skyboxTexturePath_ = "";
	std::vector<struct TextureReturnVals> textureFaces;
};

