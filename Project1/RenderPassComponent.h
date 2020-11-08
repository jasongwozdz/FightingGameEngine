#pragma once
#include <vulkan/vulkan.h>

class DepthBufferComponent;

class RenderPassComponent
{
public:

	RenderPassComponent(VkDevice& rm_logicalDevice, DepthBufferComponent& depthComponent);

	~RenderPassComponent();

	VkRenderPass m_renderPass;
private:

	VkDevice& rm_logicalDevice;
};

