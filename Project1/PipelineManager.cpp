#include "PipelineManager.h"

template <> PipelineManager* Singleton<PipelineManager>::msSingleton = 0;


PipelineManager* PipelineManager::getSingletonPtr()
{
	return msSingleton;
}

PipelineManager& PipelineManager::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

PipelineManager::PipelineManager()
{
	graphicsPipelines.fill(nullptr);
	descriptorLayouts.fill(nullptr);
}

PipelineManager::~PipelineManager()
{
	Renderer& renderer = Renderer::getSingleton();
	for (uint32_t index : pipelinesToDelete)
	{
		delete graphicsPipelines[index];
		vkDestroyDescriptorSetLayout(renderer.getLogicalDevice(), descriptorLayouts[index], nullptr);
	}
}

GraphicsPipeline* PipelineManager::createOrGetPipeline(PipelineTypes type, VkDescriptorSetLayout& layout)
{
	Renderer& renderer = Renderer::getSingleton();
	if (graphicsPipelines[type] != nullptr)
	{
		layout = descriptorLayouts[type];
		return graphicsPipelines[type];
	}
	switch (type)
	{
	case PipelineTypes::BASIC_PIPELINE:
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(renderer.getLogicalDevice(), &layoutInfo, nullptr, &descriptorLayouts[PipelineTypes::BASIC_PIPELINE]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		graphicsPipelines[BASIC_PIPELINE] = (new GraphicsPipeline(renderer.getLogicalDevice(), renderer.getRenderPass(), descriptorLayouts[PipelineTypes::BASIC_PIPELINE], renderer.getSwapChainExtent(), renderer.getDepthBufferComp()));

		pipelinesToDelete.push_back(BASIC_PIPELINE);

		break;
	}
	case PipelineTypes::DEBUG_PIPELINE:
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(renderer.getLogicalDevice(), &layoutInfo, nullptr, &descriptorLayouts[PipelineTypes::DEBUG_PIPELINE]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		graphicsPipelines[PipelineTypes::DEBUG_PIPELINE] = new GraphicsPipeline(renderer.getLogicalDevice(), renderer.getRenderPass(), descriptorLayouts[PipelineTypes::DEBUG_PIPELINE], renderer.getSwapChainExtent(), "shaders/vert.spv", "shaders/frag.spv", renderer.getDepthBufferComp(), false);

		pipelinesToDelete.push_back(DEBUG_PIPELINE);

		break;
	}
	case PipelineTypes::LINE_PIPELINE:
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(renderer.getLogicalDevice(), &layoutInfo, nullptr, &descriptorLayouts[PipelineTypes::LINE_PIPELINE]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		graphicsPipelines[PipelineTypes::LINE_PIPELINE] = new GraphicsPipeline(renderer.getLogicalDevice(), renderer.getRenderPass(), descriptorLayouts[PipelineTypes::LINE_PIPELINE], renderer.getSwapChainExtent(), "shaders/vert.spv", "shaders/frag.spv", renderer.getDepthBufferComp(), false, 1);

		pipelinesToDelete.push_back(LINE_PIPELINE);

		break;
	}
	default:
		std::cerr << "ERROR : Undefined Pipeline type.  Pipeline not created" << std::endl;
		return nullptr;
	}

	layout = descriptorLayouts[type];
	return graphicsPipelines[type];
}
