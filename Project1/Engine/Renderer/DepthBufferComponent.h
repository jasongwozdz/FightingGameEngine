#pragma once
#include <vulkan/vulkan.h>

class DepthBufferComponent
{
private:
	int WIDTH = 1200;
	int HEIGHT = 900;

	VkDevice& rm_logicalDevice;
	VkPhysicalDevice& rm_physicalDevice;
	VkCommandPool& rm_commandPool;
	VkFormat m_depthFormat;

public:

	DepthBufferComponent(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkFormat depthFormat);
	
	~DepthBufferComponent();

	//Member Variables
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;
	VkAttachmentDescription m_depthAttachment{};
	VkAttachmentReference m_depthAttachmentRef{};

};

