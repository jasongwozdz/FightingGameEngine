#pragma once
#include "Renderer/RenderPassComponent.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_vulkan.h" 

class UserInterface
{
public:
	UserInterface(GLFWwindow* window);
	~UserInterface();
	void renderFrame(VkCommandBuffer& currentBuffer); // called form inside renderer
	void prepareFrame(); //called from engine
private:

	VkPipeline m_pipeline;
	RenderPassComponent* m_renderPass;
	DepthBufferComponent* m_depthBufferComponent;
};
