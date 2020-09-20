#pragma once
#include <vulkan/vulkan.h>

class DepthBuffer
{
private:
	int WIDTH = 1200;
	int HEIGHT = 900;

	VkDevice& rm_logicalDevice;
	VkPhysicalDevice& rm_physicalDevice;
	VkCommandPool& rm_commandPool;
	VkFormat m_depthFormat;

public:

	DepthBuffer(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkFormat depthFormat);
	
	~DepthBuffer();

	//Member Variables
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;
	VkAttachmentDescription m_depthAttachment{};
	VkAttachmentReference m_depthAttachmentRef{};

};

