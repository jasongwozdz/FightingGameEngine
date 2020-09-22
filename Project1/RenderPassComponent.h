#pragma once
#include <vulkan/vulkan.h>
#include "DepthBuffer.h"

class RenderPassComponent
{
public:

	RenderPassComponent(VkDevice& rm_logicalDevice, DepthBuffer& depthComponent);

	~RenderPassComponent();

	VkRenderPass m_renderPass;
private:

	VkDevice& rm_logicalDevice;
};

