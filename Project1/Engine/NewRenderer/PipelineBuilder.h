#pragma once
#include <iostream>
#include <NewRenderer/vkinit.h>
#include <vector>

namespace PipelineBuilder
{
	struct PipelineResources
	{
		VkDevice& logicalDevice_;
		VkPipeline pipeline_;
		VkPipelineLayout pipelineLayout_;

		PipelineResources(VkDevice& logicalDevice) :
			logicalDevice_(logicalDevice)
		{};

		~PipelineResources()
		{
			std::cout << "destroy PipelineResources" << std::endl;
			vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
			vkDestroyPipeline(logicalDevice_, pipeline_, nullptr);
		}
	};

	PipelineResources* createPipeline(VkDevice& logicalDevice, VkRenderPass& renderPass, std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkExtent2D& windowExtent, VkDescriptorSetLayout& layout, bool depthEnabled, bool cullingEnabled, bool isLine = false);
}
