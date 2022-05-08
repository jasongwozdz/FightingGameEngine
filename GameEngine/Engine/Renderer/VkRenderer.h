#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <map>
#include "../Singleton.h"
#include "../Window.h"
#include "../Renderer/Textured.h"
#include "../Renderer/PipelineBuilder.h"
#include "../Renderer/UIInterface.h"
//#include "Renderable.h"
#include "RenderSubsystemInterface.h"
#include "VkTypes.h"
#include "Asset/AssetInstance.h"
#include "../Scene/Components/LightSource.h"

class DebugDrawManager;

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



class VkRenderer : Singleton<VkRenderer>
{
public:
	static VkRenderer& getSingleton();
	static VkRenderer* getSingletonPtr();
	VkRenderer(Window& window);
	void init();
	void draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights);
	void cleanup();
	void uploadMesh(Renderable* mesh);
	void uploadStaticMeshData(Renderable* mesh);

	VmaAllocator& getAllocator() { allocator_; }

	template<typename VertexType>
	void uploadStaticMeshData(std::vector<VertexType> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer);
	
	void uploadTextureData(Textured* texture);
	void prepareFrame();
	void frameBufferResizeCallback(Events::FrameBufferResizedEvent& event);
	void updateUniformBuffer(Renderable& mesh);
	VkShaderModule createShaderModule(std::string shaderPath);
	std::vector<char> readShaderFile(const std::string& filename);
	TextureResources createTextureResources(int textureWidth, int textureHeight, int numChannles, int offset, std::vector<unsigned char>& pixles, VkImageCreateInfo& textureInfo);


	template<typename UniformDataType>
	void uploadDynamicData(AssetInstance* assetInstance);
	
	template<class T> T* addRenderSubsystem()
	{
		T* renderSubsystem = new T(logicalDevice_, renderPass_, allocator_, descriptorPool_, *this);
		renderSubsystems_.push_back(renderSubsystem);
		return renderSubsystem;
	}

public:
	Window& window_;
	UI::UIInterface* ui_;
	DebugDrawManager* debugDrawManager_;

	uint32_t width_;
	uint32_t height_;
	bool recreateSwapchain_;
	int frameNumber_ = 0;


	//Vulkan Stuff
	struct
	{
		VkImageView depthImageView_;
		VkImage image_;
		VkFormat imageFormat_ = VK_FORMAT_D32_SFLOAT;
		VmaAllocation mem_;
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
	VkDescriptorPool descriptorPool_;
	std::vector<VkDescriptorSetLayout> descriptorLayouts_;
	std::vector<RenderSubsystemInterface*> renderSubsystems_;
	
private:
	void createDefaultRenderPass();
	void createDefualtDepthResources();
	void createDefaultFramebuffers();
	void createSwapchainResources();
	void createSynchronizationResources();
	void drawAssetInstances(VkCommandBuffer currentCommandBuffer, int imageIndex, const std::vector<AssetInstance*> &assetInstancesToDraw);
	void createDescriptorSet(Renderable* object);
	void createTextureResources(Renderable& o, Textured& texture);
	void uploadGraphicsCommand(std::function<void(VkCommandBuffer cmd)>&& func);
	void recreateSwapchain();
	void cleanupSwapchain();
	void createGlobalUniformBuffers();
	void uploadGlobalUniformData(int imageIndex, const DirLight& dirLight, const std::vector<PointLight>& pointLights);

	template<typename UniformDataType>
	void createUniformBuffers(AssetInstance* assetInstance);

	template<typename UniformDataType>
	void allocateDescriptorSet(AssetInstance* assetInstance);

	void createPipelineNew(AssetInstance* assetInstance);

private:
	std::unordered_map<std::string, VkShaderModule> shaderMap_;//maps vertex shader file location to its VKShaderModule
	std::unordered_map<PipelineCreateInfo, PipelineResources*, PipelineCreateInfoHash> pipelineMap_;

	struct GlobalUniformData
	{
		alignas(16) glm::vec3 viewPos;
		DirLight::DirLightUniformData dirLightData;
		PointLight::PointLightUniformData pointLightData;
	};
	GlobalUniformData globalUniformData_;
	std::vector<VulkanBuffer> globalUniformBuffer_;
};
