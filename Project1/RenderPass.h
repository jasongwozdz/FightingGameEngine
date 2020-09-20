#pragma once
#include <vulkan/vulkan.h>
#include "DepthBuffer.h"

class RenderPass
{
public:

	RenderPass(VkDevice& rm_logicalDevice, DepthBuffer& depthComponent);

	~RenderPass();

	VkRenderPass m_renderPass;
private:

	VkDevice& rm_logicalDevice;
};

