#pragma once
#include <vector>
#include <array>
#include "Renderer.h"
#include "GraphicsPipeline.h"
#include "Singleton.h"

enum PipelineTypes 
{
	BASIC_PIPELINE,
	LINE_PIPELINE,
	DEBUG_PIPELINE,
	NUM_PIPELINE_TYPES = 3
};

class PipelineManager : Singleton<PipelineManager>
{
public:
	PipelineManager();
	~PipelineManager();
	GraphicsPipeline* createOrGetPipeline(PipelineTypes type, VkDescriptorSetLayout& layout);
	static PipelineManager& getSingleton();
	static PipelineManager* getSingletonPtr();
private:
	std::vector<uint32_t> pipelinesToDelete;
	std::array<GraphicsPipeline*, PipelineTypes::NUM_PIPELINE_TYPES> graphicsPipelines;
	std::array<VkDescriptorSetLayout, PipelineTypes::NUM_PIPELINE_TYPES> descriptorLayouts;
};

