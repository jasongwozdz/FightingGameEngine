#pragma once
#include <vulkan/vulkan.h>
#include <Vector>

class GraphicsPipeline
{
public:

	GraphicsPipeline(VkDevice& logicalDevice, VkRenderPass& renderPassComponent, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, bool depthEnabled);

	GraphicsPipeline(VkDevice& logicalDevice, VkRenderPass& renderPassComponent, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, std::string vertexShader, std::string fragmentShader, bool depthEnabled);
	
	GraphicsPipeline(VkDevice& logicalDevice, VkRenderPass& renderPassComponent, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, std::string vertexShader, std::string fragmentShader, bool depthEnabled, bool cullingEnabled, float lineWidth);

	~GraphicsPipeline();

	VkPipeline m_pipeline;

	VkPipelineLayout m_pipelineLayout;

	VkRenderPass& renderPass_;

	static std::vector<char> readShaderFile(const std::string& filename);

	std::vector<VkFramebuffer> frameBuffers;

private:


	VkDescriptorSetLayout& rm_descriptorSetLayout;
	
	VkExtent2D& rm_swapChainExtent;

	VkDevice& rm_logicalDevice;

	VkShaderModule createShaderModule(const std::vector<char>& code);
};

