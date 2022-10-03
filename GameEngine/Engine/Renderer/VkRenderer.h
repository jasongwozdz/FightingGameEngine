#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <map>
#include "../Singleton.h"
#include "../Renderer/Textured.h"
#include "../Renderer/PipelineBuilder.h"
#include "../Renderer/UIInterface.h"
//#include "Renderable.h"
#include "RenderSubsystemInterface.h"
#include "VkTypes.h"
#include "Asset/AssetInstance.h"
#include "../Scene/Components/LightSource.h"
#include "RendererInterface.h"

class VulkanDebugDrawManager;

#define VK_CHECK(x) \
	do \
	{	\
		VkResult err = x;	\
		if (err)	\
		{	\
			std::cout << "ERROR: Vulkan error : " << err << std::endl;	\
			abort();	\
		} 	\
	} while(0)



class VkRenderer : public RendererInterface
{
public:
	static VkRenderer* getInstance();

	VkRenderer(Window& window);
	void init();
	void draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights);
	void cleanup();
	void uploadMesh(Renderable* mesh);
	//void uploadStaticMeshData(Renderable* mesh);

	VmaAllocator& getAllocator() { allocator_; }

	template<typename VertexType>
	void uploadStaticMeshData(std::vector<VertexType> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer);
	
	void uploadTextureData(Textured* texture);
	void prepareFrame();
	void frameBufferResizeCallback(Events::FrameBufferResizedEvent& event);
	VkShaderModule createShaderModule(std::string shaderPath);
	void deleteDynamicAssetData(Entity* assetInstance);

	template<typename UniformDataType>
	void uploadDynamicData(AssetInstance* assetInstance);

	virtual void setDefaultShader(AssetInstance* assetInstance) const;

	virtual const std::string& getParticleVertexShader(ParticleTypeVertex vertex) const;
	virtual const std::string& getParticleFragmentShader(ParticleTypeFragment fragment) const;
	
	template<class T> T* addRenderSubsystem()
	{
		T* renderSubsystem = new T(logicalDevice_, renderPass_, allocator_, debugDescriptorPool_, *this);
		renderSubsystems_.push_back(renderSubsystem);
		return renderSubsystem;
	}

public:
	UI::UIInterface* ui_;

	uint32_t width_;
	uint32_t height_;
	bool recreateSwapchain_;
	int frameNumber_ = 0;


	//Vulkan Stuff
	struct DepthResources
	{
		TextureResources imageResources_;
		VkFormat imageFormat_ = VK_FORMAT_D32_SFLOAT;
	}depthResources_;


	struct
	{
		VkCommandPool cmdPool_;
		VkFence uploadFence_;
	}uploadContext_;

	struct
	{
		std::vector<VkImage> swapChainImages_;
		std::vector<VkImageView> swapChainImageViews_;
		VkFormat imageFormat_;
		int imageCount_;
	}swapChainResources_;

	enum Face
	{
		FACE_FORWARD,
		FACE_BACKWARD,
		FACE_LEFT,
		FACE_RIGHT,
		FACE_TOP,
		FACE_BOTTOM,

		FACE_MAX
	};

	VkExtent2D windowExtent_;
	VkDevice logicalDevice_;
	VkRenderPass renderPass_;
	VkInstance instance_;
	VkDebugUtilsMessengerEXT debugMessenger_;
	VkSurfaceKHR surface_;
	VkPhysicalDevice physicalDevice_;
	VkQueue graphicsQueue_;
	uint32_t graphicsQueueFamiliy_;
	VkCommandPool cmdPool_;
	VkCommandBuffer cmdBuffer_;
	VkSwapchainKHR swapchain_;
	VmaAllocator allocator_;
	std::vector<VkFramebuffer> frameBuffers_;
	VkFence renderFence_;
	VkSemaphore presentSemaphore_;
	VkSemaphore renderSemaphore_;
	std::vector<VkDescriptorPool> descriptorPools_;
	VkDescriptorPool uiDescriptorPool_;
	VkDescriptorPool debugDescriptorPool_;
	std::vector<VkDescriptorSetLayout> descriptorLayouts_;
	std::vector<RenderSubsystemInterface*> renderSubsystems_;
	
private:
	void createOffscreenResources();
	void createOffscreenRenderPass();
	void createOffscreenFramebuffer();

	void prepareOffscreenDirectionalLight(const DirLight& dirLight, int imageIndex);
	void drawOffscreenDirLight(const DirLight& dirLight, int imageIndex, const std::vector<AssetInstance*>& assetInstancesToDraw);

	TextureResources createTextureResources(int textureWidth, int textureHeight, int numChannles, int offset, std::vector<unsigned char>& pixles, VkImageCreateInfo& textureInfo);


	void setImageLayout(VkImage& image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void setImageLayout(VkImage& image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, const VkImageSubresourceRange& subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void setImageLayoutNoUpload(VkCommandBuffer cmd, VkImage& image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void setImageLayoutNoUpload(VkCommandBuffer cmd, VkImage& image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, const VkImageSubresourceRange& subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	
	void prepareAssetInstanceData(const std::vector<AssetInstance*>& assetInstancesToDraw, int imageIndex);
	void createDefaultRenderPass();
	void createDepthImageResources(TextureResources& vulkanImage, const VkExtent3D& extent, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, bool createSampler = false);
	void createDefaultFramebuffers();
	void createSwapchainResources();
	void createDescriptorPools();
	void createSynchronizationResources();
	void drawAssetInstances(VkCommandBuffer currentCommandBuffer, int imageIndex, const std::vector<AssetInstance*> &assetInstancesToDraw);
	void createTextureResources(Renderable& o, Textured& texture);
	void uploadGraphicsCommand(std::function<void(VkCommandBuffer cmd)>&& func);
	void recreateSwapchain();
	void cleanupSwapchain();
	void createGlobalUniformBuffers();
	void uploadGlobalUniformData(int imageIndex, const DirLight& dirLight, const std::vector<PointLight>& pointLights);

	void drawOffscreenPointLightFace(const PointLight& pointLight, Face face, int imageIndex, const std::vector<AssetInstance*>& assetInstancesToDraw);

	template<typename UniformDataType>
	void createUniformBuffers(AssetInstance* assetInstance);

	void createDescriptorSetLayout(const PipelineCreateInfo & pipelineCreateInfo, VkDescriptorSetLayout& outDescriptorSetLayout);

	void allocateDescriptorSet(AssetInstance* assetInstance, int imageIndex);
	void allocateOffscreenDescriptorSet(AssetInstance* assetInstance, int imageIndex);

	void createPipelineNew(AssetInstance* assetInstance);

	void createPipelineNew(const PipelineCreateInfo& createInfo, PipelineResources*& pipelineResources);

	void createCubeMapImage(TextureResources& textureResources, const uint32_t width, const uint32_t height, VkImageUsageFlags imageUsage, VkFormat imageFormat);
	void createSampledImage(TextureResources& textureResources, const uint32_t width, const uint32_t height, VkImageUsageFlags imageUsage, VkFormat imageFormat);

private:
	std::unordered_map<std::string, VkShaderModule> shaderMap_;//maps vertex shader file location to its VKShaderModule
	//std::unordered_map<PipelineCreateInfo, PipelineResources*, PipelineCreateInfoHash> pipelineMap_;

#define MAX_RENDERED_OBJECTS 1000
	std::vector<PipelineResources*> pipelines_;

	typedef int PipelinesIndex;
	std::unordered_map<PipelineCreateInfo, PipelinesIndex, PipelineCreateInfoHash> pipelineResourcesMap_;

	std::unordered_map<PipelineCreateInfo, VkDescriptorSetLayout, PipelineCreateInfoHash> descriptorSetLayoutMap_;

	std::unordered_map<PipelineCreateInfo, std::vector<AssetInstance*>, PipelineCreateInfoHash> pipelineAssetMap_;


	std::vector<VulkanBuffer> globalUniformBuffer_;
	std::vector<Entity*> assetInstancesToDelete_;

	struct
	{
		TextureResources offscreenTextureResources_;
		TextureResources sampledImage;
		//TextureResources shadowCubeMap_;
		VkRenderPass offscreenRenderPass_;
		std::vector<VkFramebuffer> offscreenFrameBuffers_;
		DepthResources offscreenDepthResources_;
		PipelineResources* offscreenPipeline_;
		struct
		{
			alignas(16) glm::mat4 pointLightSpaceMatrix;
			alignas(16) glm::mat4 projection;
			alignas(16) glm::vec3 lightPos;
		} globalOffscreenUniformData_;

		std::vector<VulkanBuffer> offscreenUniformBuffers_;
	}offscreenResources_;
};
