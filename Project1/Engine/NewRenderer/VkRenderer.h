#pragma once
#include <Vector>
#include <map>
#include <Singleton.h> 
#include <NewRenderer/Renderable.h>
#include <NewRenderer/Textured.h>
#include <Window.h>
#include <NewRenderer/PipelineBuilder.h>
#include <libs/VulkanMemoryAllocator/vk_mem_alloc.h>
#include <NewRenderer/VkTypes.h>
#include <NewRenderer/UIInterface.h>

enum PipelineTypes 
{
	BASIC_PIPELINE,
	LINE_PIPELINE,
	DEBUG_PIPELINE,
	ANIMATION_PIPELINE,
	NUM_PIPELINE_TYPES = 4
};

struct TextureResources
{
	VulkanImage image_;
	VkImageView view_;
	VkSampler sampler_;
};

struct RenderableObject
{
	VkBuffer vertexBuffer;
	VmaAllocation vertexMem;

	VkBuffer indexBuffer;
	VmaAllocation indexMem;

	std::vector<VkBuffer> uniformBuffer;
	std::vector<VmaAllocation> uniformMem;

	PipelineTypes pipelineType;

	Renderable* renderable_;

	VkDescriptorSetLayout descriptorLayout_;
	std::vector<VkDescriptorSet> descriptorSets_;

	TextureResources textureResources_;
};

class VkRenderer : Singleton<VkRenderer>
{
public:
	void init(Window& window);
	void draw();
	void cleanup();
	void uploadObject(Renderable* renderableObject);
	void uploadObject(Renderable* renderableObject, Textured* texture);
	void prepareFrame();

	static VkRenderer& getSingleton();
	static VkRenderer* getSingletonPtr();
	
	uint32_t width_;
	uint32_t height_;

	VkExtent2D windowExtent_;

	VkDevice logicalDevice_;

	VkRenderPass renderPass_;

private:
	UIInterface* ui_;

	VkInstance instance_;

	VkDebugUtilsMessengerEXT debugMessenger_;

	VkSurfaceKHR surface_;

	VkPhysicalDevice physicalDevice_;

	VkQueue graphicsQueue_;
	uint32_t graphicsQueueFamiliy_;
	
	VkCommandPool cmdPool_;
	VkCommandBuffer cmdBuffer_;

	VkRenderPass mainRenderPass_;

	VkSwapchainKHR swapchain_;

	VmaAllocator allocator_;

	std::vector<VkFramebuffer> frameBuffers_;
	
	int frameNumber_ = 0;

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
		std::vector<VkFramebuffer> swapChainFramebuffers_;
		VkFormat imageFormat_;
		int imageCount_;
	}swapChainResources_;

	VkFence renderFence_;
	VkSemaphore presentSemaphore_;
	VkSemaphore renderSemaphore_;

	std::vector<RenderableObject> renderObjects_;

	std::vector<PipelineBuilder::PipelineResources*> pipelines_;

	std::map<PipelineTypes, std::vector<RenderableObject*>> pipelineMap_;

	VkDescriptorPool descriptorPool_;
	std::vector<VkDescriptorSetLayout> descriptorLayouts_;
	
	void createDefaultRenderPass();

	void createDefualtDepthResources();

	void createDefaultFramebuffers();

	void createSwapchainResources();

	void createSynchronizationResources();

	void updateUniformBuffer(RenderableObject mesh);

	void drawObjects(VkCommandBuffer currentCommandBuffer, int imageIndex);

	void initPipelines();

	void createDescriptorSet(RenderableObject& object);

	void createTextureResources(RenderableObject& o, Textured& texture);

	VkShaderModule createShaderModule(const std::vector<char>& code);
	
	std::vector<char> readShaderFile(const std::string& filename);

	void uploadGraphicsCommand(std::function<void(VkCommandBuffer cmd)>&& func);
};
