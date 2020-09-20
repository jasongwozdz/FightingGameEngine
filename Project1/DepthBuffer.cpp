#include "DepthBuffer.h"
#include "VulkanImage.h"

DepthBuffer::DepthBuffer(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkFormat depthFormat) : 
	rm_logicalDevice(logicalDevice),
	rm_physicalDevice(physicalDevice),
	rm_commandPool(commandPool),
	m_depthFormat(depthFormat) 
{
	VulkanImage::createImage(rm_logicalDevice, rm_physicalDevice, rm_commandPool, WIDTH, HEIGHT, VK_SAMPLE_COUNT_1_BIT, m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
	m_depthImageView = VulkanImage::createImageView(rm_logicalDevice, m_depthImage, m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	m_depthAttachment.format = m_depthFormat;
	m_depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	m_depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	m_depthAttachmentRef.attachment = 1;
	m_depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

DepthBuffer::~DepthBuffer() {
	vkFreeMemory(rm_logicalDevice, m_depthImageMemory, nullptr);
	vkDestroyImage(rm_logicalDevice, m_depthImage, nullptr);
	vkDestroyImageView(rm_logicalDevice, m_depthImageView, nullptr);
}

