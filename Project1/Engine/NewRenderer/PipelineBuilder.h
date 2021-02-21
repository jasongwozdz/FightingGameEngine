#pragma once
#include <NewRenderer/vkinit.h>
#include <vector>

namespace PipelineBuilder
{
	struct PipelineResources
	{
		VkPipeline pipeline_;
		VkPipelineLayout pipelineLayout_;
	};

	PipelineResources* createPipeline(VkDevice& logicalDevice, VkRenderPass& renderPass, std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkExtent2D& windowExtent, VkDescriptorSetLayout& layout, bool depthEnabled, bool cullingEnabled, bool isLine = false);
}
