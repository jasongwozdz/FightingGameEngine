#pragma once
#include <vulkan/vulkan.h>
#include <Vector>

class GraphicsPipeline
{
public:

	GraphicsPipeline(VkDevice& logicalDevice, VkRenderPass& renderPass, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, bool depthEnabled);

	~GraphicsPipeline();

	VkPipeline m_pipeline;

	VkPipelineLayout m_pipelineLayout;
private:
	
	VkDescriptorSetLayout& rm_descriptorSetLayout;
	
	VkExtent2D& rm_swapChainExtent;

	VkDevice& rm_logicalDevice;

	VkRenderPass& rm_renderPass;

	std::vector<char> readShaderFile(const std::string& filename);

	VkShaderModule createShaderModule(const std::vector<char>& code);
};

