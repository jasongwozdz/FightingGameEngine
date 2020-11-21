#pragma once
#include "RenderPassComponent.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h" 

class UserInterfaceImp
{
public:
	UserInterfaceImp(GLFWwindow* window);
	~UserInterfaceImp();
	void renderFrame(VkCommandBuffer& currentBuffer); // called form inside renderer
	void prepareFrame(); //called from engine
private:
	struct {
		VkDeviceSize vertexBufferSize;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer vertexBuffer;
		VkDeviceSize indexBufferSize;
		VkDeviceMemory indexBufferMemory;
		VkBuffer indexBuffer;
	} uiBuffers;

	VkPipeline m_pipeline;
	RenderPassComponent* m_renderPass;
	DepthBufferComponent* m_depthBufferComponenet;
};
