#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <map>
#include "../Singleton.h"
#include "../Window.h"
#include "../NewRenderer/Textured.h"
#include "../NewRenderer/PipelineBuilder.h"
#include "../NewRenderer/UIInterface.h"
#include "Renderable.h"

class DebugDrawManager; // forware declare this class to remove cyclical dependency between DebugDrawManager.h->Scene.h->VkRenderer.h

class VkRenderer : Singleton<VkRenderer>
{
public:
	VkRenderer(Window& window);
	void init();
	void draw(std::vector<Renderable*>& objectsToDraw);
	void cleanup();
	void uploadObject(Renderable* renderableObject);
	void uploadObject(Renderable* renderableObject, Textured* texture, bool animated = false);
	void prepareFrame();
	void frameBufferResizeCallback(Events::FrameBufferResizedEvent& event);
	void updateUniformBuffer(Renderable& mesh);

	VkShaderModule createShaderModule(const std::vector<char>& code);
	std::vector<char> readShaderFile(const std::string& filename);

	static VkRenderer& getSingleton();
	static VkRenderer* getSingletonPtr();
	
	uint32_t width_;
	uint32_t height_;

	VkExtent2D windowExtent_;

	VkDevice logicalDevice_;

	VkRenderPass renderPass_;

	Window& window_;

	UI::UIInterface* ui_;
	
	DebugDrawManager* debugDrawManager_;

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

	bool recreateSwapchain_;

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
		VkFormat imageFormat_;
		int imageCount_;
	}swapChainResources_;

	VkFence renderFence_;
	VkSemaphore presentSemaphore_;
	VkSemaphore renderSemaphore_;

	std::vector<PipelineBuilder::PipelineResources*> pipelines_;

	//std::map<PipelineTypes, std::vector<RenderableObject*>> pipelineMap_;

	VkDescriptorPool descriptorPool_;
	std::vector<VkDescriptorSetLayout> descriptorLayouts_;
	
	void createDefaultRenderPass();

	void createDefualtDepthResources();

	void createDefaultFramebuffers();

	void createSwapchainResources();

	void createSynchronizationResources();

	void drawObjects(VkCommandBuffer currentCommandBuffer, int imageIndex, std::vector<Renderable*>& objectsToDraw);

	void initPipelines();

	void createDescriptorSet(Renderable* object);

	void createTextureResources(Renderable& o, Textured& texture);


	void uploadGraphicsCommand(std::function<void(VkCommandBuffer cmd)>&& func);

	void recreateSwapchain();

	void cleanupSwapchain();
};
