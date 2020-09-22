#pragma once
#include <vulkan/vulkan.h>
#include <Vector>

class RenderPassComponent;
class DepthBuffer;


class GraphicsPipeline
{
public:

	GraphicsPipeline(VkDevice& logicalDevice, RenderPassComponent& renderPassComponent, VkDescriptorSetLayout& descriptorSetLayout, VkExtent2D& swapChainExtent, DepthBuffer* depthComponent = nullptr);

	~GraphicsPipeline();

	VkPipeline m_pipeline;

	VkPipelineLayout m_pipelineLayout;

	DepthBuffer* m_depthComponent;
	
	RenderPassComponent& rm_renderPassComponent;


private:

	VkDescriptorSetLayout& rm_descriptorSetLayout;
	
	VkExtent2D& rm_swapChainExtent;

	VkDevice& rm_logicalDevice;

	std::vector<char> readShaderFile(const std::string& filename);

	VkShaderModule createShaderModule(const std::vector<char>& code);
};

