#pragma once
#include <vulkan/vulkan.h>
#include <Vector>

class RenderPassComponent;
class DepthBufferComponent;

class GraphicsPipeline
{
public:

	GraphicsPipeline(VkDevice& logicalDevice, RenderPassComponent& renderPassComponent, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, DepthBufferComponent* depthComponent = nullptr);

	~GraphicsPipeline();

	VkPipeline m_pipeline;

	VkPipelineLayout m_pipelineLayout;

	DepthBufferComponent* m_depthComponent;
	
	RenderPassComponent& rm_renderPassComponent;

	static std::vector<char> readShaderFile(const std::string& filename);

private:

	VkDescriptorSetLayout& rm_descriptorSetLayout;
	
	VkExtent2D& rm_swapChainExtent;

	VkDevice& rm_logicalDevice;

	VkShaderModule createShaderModule(const std::vector<char>& code);
};

