#include "VkRenderer.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "PipelineBuilder.h"
#include "../libs/vk-bootstrap/VkBootstrap.h"
#include "vkinit.h"
#include "../DebugDrawManager.h"
#define VMA_IMPLEMENTATION
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "GLFW/glfw3.h"
#include "Asset/AssetInstance.h"
#include "../Vertex.h"
#include "../Scene/Components/Camera.h"
#include "../Console/Console.h"
#include "../EngineSettings.h"
#include "Vulkan/VulkanDebugDrawManager.h"
#include "../Particles/ParticleTypes.h"

#define SHADOW_WIDTH_HEIGHT 1024

VkRenderer* VkRenderer::getInstance()
{
	RendererInterface* renderer = RendererInterface::getSingletonPtr();
	if (renderer->api_ == RendererInterface::RenderAPI::VULKAN)
	{
		return static_cast<VkRenderer*>(renderer);
	}
	else
	{
		_ASSERTE(false, ("Trying to get Vulkan renderer but Opengl api was selected"));
	}
	return nullptr;
}

VkRenderer::VkRenderer(Window& window) :
	RendererInterface(&window, RenderAPI::VULKAN)
{
	Console::addBoolVar("shadows.enabled", true , nullptr);
}

void VkRenderer::frameBufferResizeCallback(Events::FrameBufferResizedEvent& event)
{
	width_ = event.width_;
	height_ = event.height_;
	windowExtent_ = { width_, height_ };
	std::vector<Entity*> assetInstances;
	Scene::getSingleton().getAllEntitiesWithComponents<AssetInstance>(assetInstances);
	for (auto iter : assetInstances)
	{
		AssetInstance& assetInstance = iter->getComponent<AssetInstance>();
		assetInstance.createInfo_.windowExtent = windowExtent_;
	}

	recreateSwapchain_ = true;
}

void VkRenderer::cleanupSwapchain()
{
	for (auto framebuffer : frameBuffers_) {
		vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
	}

	for (auto imageViews : swapChainResources_.swapChainImageViews_)
	{
		vkDestroyImageView(logicalDevice_, imageViews, nullptr);
	}

	vkDestroyImage(logicalDevice_, depthResources_.imageResources_.image_.image_, nullptr);
	vkDestroyImageView(logicalDevice_, depthResources_.imageResources_.view_, nullptr);

	vkFreeCommandBuffers(logicalDevice_, cmdPool_, 1, &cmdBuffer_);

	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

	vkDestroySwapchainKHR(logicalDevice_, swapchain_, nullptr);

	//delete ui_;
}

void VkRenderer::createGlobalUniformBuffers()
{
	size_t uniformBufferSize = sizeof(GlobalUniformData);
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = uniformBufferSize;
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	globalUniformBuffer_.resize(swapChainResources_.imageCount_);
	for (int i = 0; i < swapChainResources_.imageCount_; i++)
	{
		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &globalUniformBuffer_[i].buffer_, &globalUniformBuffer_[i].mem_, nullptr));
	}
}

void VkRenderer::uploadGlobalUniformData(int imageIndex, const DirLight& dirLight, const std::vector<PointLight>& pointLights)
{
	globalUniformData_.dirLightData = dirLight.uniformData_;

	globalUniformData_.numLights = pointLights.size();
	for (int i = 0; i < globalUniformData_.numLights; i++)
	{
		globalUniformData_.pointLightData[i] = pointLights[i].uniformData_;
	}

	globalUniformData_.viewPos = Scene::getSingleton().getCurrentCamera().entity_->getComponent<Transform>().position_;

	void* data;
	vmaMapMemory(allocator_, globalUniformBuffer_[imageIndex].mem_, &data);
	memcpy(data, &globalUniformData_, sizeof(GlobalUniformData));
	vmaUnmapMemory(allocator_, globalUniformBuffer_[imageIndex].mem_);
}

void VkRenderer::drawOffscreenDirLight(const DirLight& dirLight, int imageIndex, const std::vector<AssetInstance*>& assetInstancesToDraw)
{
	VkClearValue clearValues;
	clearValues.depthStencil = { 1.0f, 0 };

	VkDeviceSize offsets[1] = { 0 };

	VkRenderPassBeginInfo offscreenRpInfo = vkinit::renderpass_begin_info(offscreenResources_.offscreenRenderPass_, { SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT }, offscreenResources_.offscreenFrameBuffers_[imageIndex]);
	offscreenRpInfo.clearValueCount = 1;
	offscreenRpInfo.pClearValues = &clearValues;
	vkCmdBeginRenderPass(cmdBuffer_, &offscreenRpInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (auto object : assetInstancesToDraw)
	{
		allocateOffscreenDescriptorSet(object, imageIndex);

		PipelineCreateInfo createInfo = object->createInfo_;
		if (!createInfo.hasSkeleton)
		{
			createInfo.vertexShader = "./shaders/offscreenTexturedMeshLighting.vert.spv";
			createInfo.fragmentShader = "./shaders/offscreenTexturedMeshLighting.frag.spv";
		}
		else
		{
			createInfo.vertexShader = "./shaders/offscreenAnimatedMeshLighting.vert.spv";
			createInfo.fragmentShader = "./shaders/offscreenAnimatedMeshLighting.frag.spv";
		}
		createInfo.isOffscreen = true;
		createInfo.cullingEnabled = false;
		PipelineResources* pipelineResources = nullptr;
		createPipelineNew(createInfo, pipelineResources);

		vkCmdBindPipeline(cmdBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipeline_);
		VkDescriptorSet& descriptorSet = object->data_.offscreenDescriptorSet_[imageIndex];
		vkCmdBindDescriptorSets(cmdBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipelineLayout_, 0, 1, &descriptorSet, 0, nullptr);

		vkCmdBindVertexBuffers(cmdBuffer_, 0, 1, &object->asset_->mesh_.vertexBuffer_.buffer_, offsets);

		vkCmdBindIndexBuffer(cmdBuffer_, object->asset_->mesh_.indexBuffer_.buffer_, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer_, static_cast<uint32_t>(object->asset_->mesh_.numInidicies_), 1, 0, 0, 0);
	}
	vkCmdEndRenderPass(cmdBuffer_);
}

void VkRenderer::deleteDynamicAssetData(Entity* assetInstance)
{
	AssetInstance& instance = assetInstance->getComponent<AssetInstance>();
	instance.toDelete = true;
	assetInstancesToDelete_.push_back(assetInstance);
}

void VkRenderer::setDefaultShader(AssetInstance * assetInstance) const
{
	bool hasSkeleton = assetInstance->getSkeleton();
	bool lightingEnabled = assetInstance->createInfo_.lightingEnabled;
	bool hasTexture = assetInstance->asset_->texture_;

	if (hasSkeleton)
	{
		assetInstance->createInfo_.hasSkeleton = true;
		assetInstance->createInfo_.hasTexture = true;
		if (!assetInstance->createInfo_.lightingEnabled)
		{
			assetInstance->createInfo_.vertexShader = "./shaders/animatedMesh.vert.spv";
			assetInstance->createInfo_.fragmentShader = "./shaders/animatedMesh.frag.spv";
		}
		else
		{
			assetInstance->createInfo_.vertexShader = "./shaders/animatedMeshLighting.vert.spv";
			assetInstance->createInfo_.fragmentShader = "./shaders/animatedMeshLighting.frag.spv";
		}
	}
	else if (hasTexture)
	{
		assetInstance->createInfo_.hasTexture = true;
		if (!assetInstance->createInfo_.lightingEnabled)
		{
			assetInstance->createInfo_.vertexShader = "./shaders/texturedMeshVert.spv";
			assetInstance->createInfo_.fragmentShader = "./shaders/texturedMeshFrag.spv";
		}
		else
		{
			assetInstance->createInfo_.vertexShader = "./shaders/texturedMeshLighting.vert.spv";
			assetInstance->createInfo_.fragmentShader = "./shaders/texturedMeshLighting.frag.spv";
		}
	}
	else if (assetInstance->createInfo_.lightingEnabled)
	{
		assetInstance->createInfo_.vertexShader = "./shaders/shaderLighting.vert.spv";
		assetInstance->createInfo_.fragmentShader = "./shaders/shaderLighting.frag.spv";
	}
	else
	{
		assetInstance->createInfo_.vertexShader = "./shaders/vert.spv";
		assetInstance->createInfo_.fragmentShader = "./shaders/frag.spv";
	}
}

const std::string& VkRenderer::getParticleVertexShader(ParticleTypeVertex vertex) const
{
	static const std::string s_particleShaderPaths[static_cast<int>(ParticleTypeVertex::MAX)] = 
	{
		"./shaders/particle.vert.spv",
	};
	return s_particleShaderPaths[static_cast<int>(vertex)];
}

const std::string& VkRenderer::getParticleFragmentShader(ParticleTypeFragment fragment) const
{
	static const std::string s_particleShaderPaths[static_cast<int>(ParticleTypeFragment::MAX)] = 
	{
		"./shaders/particle.frag.spv",
	};
	return s_particleShaderPaths[static_cast<int>(fragment)];
}

void VkRenderer::createPipelineNew(AssetInstance* assetInstance)
{
	const PipelineCreateInfo& createInfo = assetInstance->createInfo_;
	auto foundResourceMap = pipelineResourcesMap_.find(createInfo);
	if (foundResourceMap != pipelineResourcesMap_.end())
	{
		auto foundAssetMap = pipelineAssetMap_.find(createInfo);
		if (foundAssetMap != pipelineAssetMap_.end())
		{
			foundAssetMap->second.push_back(assetInstance);
		}
		else
		{
			pipelineAssetMap_.insert({ createInfo, {assetInstance} });
		}
		return;
	}

	PipelineResources* retVals = new PipelineResources(logicalDevice_);

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	if (createInfo.hasSkeleton)
	{
		bindingDescription = VertexUtil::getBindingDescription<Vertex>();
		attributeDescriptions = VertexUtil::getAttributeDescriptions<Vertex>();
	}
	else
	{
		bindingDescription = VertexUtil::getBindingDescription<NonAnimVertex>();
		attributeDescriptions = VertexUtil::getAttributeDescriptions<NonAnimVertex>();
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = createInfo.topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)createInfo.windowExtent.width;
	viewport.height = (float)createInfo.windowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = createInfo.windowExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = createInfo.cullingEnabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smooth

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	if (createInfo.depthEnabled)
	{
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	}

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	//if (createInfo.isOffscreen)
	//{
	//	colorBlendAttachment.blendEnable = VK_FALSE;
	//}
	//else
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &assetInstance->data_.descriptorLayout_;

	if (createInfo.pushConstantRanges.size() > 0)
	{
		pipelineLayoutInfo.pushConstantRangeCount = createInfo.pushConstantRanges.size();
		pipelineLayoutInfo.pPushConstantRanges = createInfo.pushConstantRanges.data();
	}
	
	VK_CHECK(vkCreatePipelineLayout(logicalDevice_, &pipelineLayoutInfo, nullptr, &retVals->pipelineLayout_));

	VkShaderModule vertexShaderModule = createShaderModule(createInfo.vertexShader);
	VkShaderModule fragmentShaderModule = createShaderModule(createInfo.fragmentShader);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo = { vertShaderStageInfo, fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStageInfo.size();
	pipelineInfo.pStages = shaderStageInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState =  &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = retVals->pipelineLayout_;
	pipelineInfo.renderPass = renderPass_;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	VK_CHECK(vkCreateGraphicsPipelines(logicalDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &retVals->pipeline_));

	pipelines_.push_back(retVals);

	const int pipelineIndex = pipelines_.size()-1;
	assetInstance->data_.pipelineIndex_ = pipelineIndex;
	pipelineAssetMap_.insert({ createInfo, {assetInstance} });
	pipelineResourcesMap_.insert({ createInfo, pipelineIndex});
}

void VkRenderer::createPipelineNew(const PipelineCreateInfo& createInfo, PipelineResources *& pipelineResources)
{
	auto found = pipelineResourcesMap_.find(createInfo);
	if(found != pipelineResourcesMap_.end())
	{
		pipelineResources = pipelines_[found->second];
		return;
	}
	PipelineResources* retVals = new PipelineResources(logicalDevice_);

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	if (createInfo.hasSkeleton)
	{
		bindingDescription = VertexUtil::getBindingDescription<Vertex>();
		attributeDescriptions = VertexUtil::getAttributeDescriptions<Vertex>();
	}
	else
	{
		bindingDescription = VertexUtil::getBindingDescription<NonAnimVertex>();
		attributeDescriptions = VertexUtil::getAttributeDescriptions<NonAnimVertex>();
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = createInfo.topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)createInfo.windowExtent.width;
	viewport.height = (float)createInfo.windowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = createInfo.windowExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = createInfo.cullingEnabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smooth

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	if (createInfo.depthEnabled)
	{
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	}

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	VkDescriptorSetLayout descriptorSetLayout;
	createDescriptorSetLayout(createInfo, descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	if (createInfo.pushConstantRanges.size() > 0)
	{
		pipelineLayoutInfo.pushConstantRangeCount = createInfo.pushConstantRanges.size();
		pipelineLayoutInfo.pPushConstantRanges = createInfo.pushConstantRanges.data();
	}
	
	VK_CHECK(vkCreatePipelineLayout(logicalDevice_, &pipelineLayoutInfo, nullptr, &retVals->pipelineLayout_));

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo;

	VkShaderModule vertexShaderModule = createShaderModule(createInfo.vertexShader);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShaderModule;
	vertShaderStageInfo.pName = "main";
	
	shaderStageInfo.push_back(vertShaderStageInfo);

	if (!createInfo.isOffscreen)
	{
		VkShaderModule fragmentShaderModule = createShaderModule(createInfo.fragmentShader);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShaderModule;
		fragShaderStageInfo.pName = "main";
		
		shaderStageInfo.push_back(fragShaderStageInfo);
	}

	if (createInfo.geometryShader.size() > 0)
	{
		VkShaderModule geometryShaderModule = createShaderModule(createInfo.geometryShader);

		VkPipelineShaderStageCreateInfo geometryShaderStageInfo{};
		geometryShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryShaderStageInfo.pName = "main";
		shaderStageInfo.push_back(geometryShaderStageInfo);
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStageInfo.size();
	pipelineInfo.pStages = shaderStageInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState =  &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = retVals->pipelineLayout_;
	pipelineInfo.renderPass = createInfo.isOffscreen ? offscreenResources_.offscreenRenderPass_ : renderPass_;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	VK_CHECK(vkCreateGraphicsPipelines(logicalDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &retVals->pipeline_));

	pipelines_.push_back(retVals);
	pipelineResourcesMap_.insert({ createInfo, pipelines_.size() - 1 });
	pipelineResources = retVals;
}

void VkRenderer::createCubeMapImage(TextureResources& textureResources, const uint32_t width, const uint32_t height, VkImageUsageFlags imageUsage, VkFormat imageFormat)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = imageFormat;
	imageCreateInfo.extent = { width, width, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = FACE_MAX;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = imageUsage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(allocator_, &imageCreateInfo, &allocInfo, &textureResources.image_.image_, &textureResources.image_.mem_, nullptr);

	// Image barrier for optimal image (target)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 6;

	setImageLayout(textureResources.image_.image_, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { width, height}, subresourceRange);

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = VK_NULL_HANDLE;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCreateInfo.format = imageFormat;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.layerCount = FACE_MAX;
	viewCreateInfo.image = textureResources.image_.image_;
	VK_CHECK(vkCreateImageView(logicalDevice_, &viewCreateInfo, nullptr, &textureResources.view_));

	// Create sampler
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
	samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 1.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK(vkCreateSampler(logicalDevice_, &samplerCreateInfo, nullptr, &textureResources.sampler_));
}

void VkRenderer::createSampledImage(TextureResources & textureResources, const uint32_t width, const uint32_t height, VkImageUsageFlags imageUsage, VkFormat imageFormat)
{
	VkExtent3D extent = { width, height, 1 };
	VkImageCreateInfo textureInfo = vkinit::image_create_info(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(allocator_, &textureInfo, &allocInfo, &textureResources.image_.image_, &textureResources.image_.mem_, nullptr);

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	int bytesPerPixel = 4;
	VkDeviceSize imageSize = width * height * bytesPerPixel;

	VkImageViewCreateInfo imageView = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, textureResources.image_.image_, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageView, nullptr, &textureResources.view_)); //needs to be deleted

	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VK_CHECK(vkCreateSampler(logicalDevice_, &samplerInfo, nullptr, &textureResources.sampler_));
}

template<typename UniformDataType>
void VkRenderer::createUniformBuffers(AssetInstance* assetInstance)
{
	size_t uniformBufferSize = sizeof(UniformDataType);
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = uniformBufferSize;
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	assetInstance->data_.uniformData_.resize(swapChainResources_.imageCount_);
	for (int i = 0; i < swapChainResources_.imageCount_; i++)
	{
		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &assetInstance->data_.uniformData_[i].buffer_, &assetInstance->data_.uniformData_[i].mem_, nullptr));
	}
}

void VkRenderer::createDescriptorSetLayout(const PipelineCreateInfo& pipelineCreateInfo, VkDescriptorSetLayout& outDescriptorSetLayout)
{
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	int bindingIndex = 0;

	auto found = descriptorSetLayoutMap_.find(pipelineCreateInfo);
	if(found != descriptorSetLayoutMap_.end())
	{
		outDescriptorSetLayout = found->second;
		return;
	}

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = bindingIndex++;
	uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	descriptorSetLayoutBindings.push_back(uboLayoutBinding);

	if (pipelineCreateInfo.hasTexture && !pipelineCreateInfo.isOffscreen)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = bindingIndex++;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorSetLayoutBindings.push_back(samplerLayoutBinding);
	}

	if (pipelineCreateInfo.lightingEnabled && !pipelineCreateInfo.isOffscreen)
	{
		VkDescriptorSetLayoutBinding uboDirLightingBinding{};
		uboDirLightingBinding.binding = bindingIndex++;
		uboDirLightingBinding.descriptorCount = 1; //number of elements in ubo array
		uboDirLightingBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboDirLightingBinding.pImmutableSamplers = nullptr;
		uboDirLightingBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorSetLayoutBindings.push_back(uboDirLightingBinding);

		VkDescriptorSetLayoutBinding pointLightSamplerBinding{};
		pointLightSamplerBinding.binding = bindingIndex++;
		pointLightSamplerBinding.descriptorCount = 1;
		pointLightSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pointLightSamplerBinding.pImmutableSamplers = nullptr;
		pointLightSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetLayoutBindings.push_back(pointLightSamplerBinding);
	}

	if (pipelineCreateInfo.isOffscreen)
	{
		VkDescriptorSetLayoutBinding globalOffscreenLightingUbo{};
		globalOffscreenLightingUbo.binding = bindingIndex++;
		globalOffscreenLightingUbo.descriptorCount = 1; //number of elements in ubo array
		globalOffscreenLightingUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalOffscreenLightingUbo.pImmutableSamplers = nullptr;
		globalOffscreenLightingUbo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		descriptorSetLayoutBindings.push_back(globalOffscreenLightingUbo);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	layoutInfo.pBindings = descriptorSetLayoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &outDescriptorSetLayout));

	descriptorSetLayoutMap_.insert({ pipelineCreateInfo, outDescriptorSetLayout});
}

void VkRenderer::allocateDescriptorSet(AssetInstance* assetInstance, int imageIndex)
{
	createDescriptorSetLayout(assetInstance->createInfo_, assetInstance->data_.descriptorLayout_);

	VkDescriptorSetLayout& currLayout = assetInstance->data_.descriptorLayout_;

	std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPools_[imageIndex];
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
	allocInfo.pSetLayouts = layouts.data();

	assetInstance->data_.descriptorSets_.resize(swapChainResources_.imageCount_);
	VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, assetInstance->data_.descriptorSets_.data()));

	int descriptorSetSize = 0;

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = assetInstance->data_.uniformData_[imageIndex].buffer_;
	bufferInfo.offset = 0;
	bufferInfo.range = assetInstance->sizeOfUniformData_;
	descriptorSetSize++;

	VkDescriptorImageInfo imageInfo{};
	if (assetInstance->createInfo_.hasTexture && !assetInstance->createInfo_.isOffscreen)
	{
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = assetInstance->asset_->texture_->resources_.view_;
		imageInfo.sampler = assetInstance->asset_->texture_->resources_.sampler_;
		descriptorSetSize++;
	}

	VkDescriptorBufferInfo lightingBufferInfo{};
	VkDescriptorImageInfo pointLightShadowMapInfo{};
	if (assetInstance->createInfo_.lightingEnabled && !assetInstance->createInfo_.isOffscreen)
	{
		lightingBufferInfo.buffer = globalUniformBuffer_[imageIndex].buffer_;
		lightingBufferInfo.offset = 0;
		lightingBufferInfo.range = sizeof(GlobalUniformData);
		descriptorSetSize++;

		pointLightShadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		pointLightShadowMapInfo.imageView = offscreenResources_.offscreenDepthResources_.imageResources_.view_;
		pointLightShadowMapInfo.sampler = offscreenResources_.offscreenDepthResources_.imageResources_.sampler_;
		descriptorSetSize++;
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites{};
	descriptorWrites.resize(descriptorSetSize);
	int descriptorWriteIdx = 0;

	descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[imageIndex];
	descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
	descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
	descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
	descriptorWrites[descriptorWriteIdx].pBufferInfo = &bufferInfo;
	descriptorWriteIdx++;

	if (assetInstance->asset_->texture_)
	{
		descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[imageIndex];
		descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
		descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
		descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
		descriptorWrites[descriptorWriteIdx].pImageInfo = &imageInfo;
		descriptorWriteIdx++;
	}

	if (assetInstance->createInfo_.lightingEnabled)
	{

		descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[imageIndex];
		descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
		descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
		descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
		descriptorWrites[descriptorWriteIdx].pBufferInfo = &lightingBufferInfo;
		descriptorWriteIdx++;

		descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[imageIndex];
		descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
		descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
		descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
		descriptorWrites[descriptorWriteIdx].pImageInfo = &pointLightShadowMapInfo;
		descriptorWriteIdx++;
	}

	vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VkRenderer::allocateOffscreenDescriptorSet(AssetInstance* assetInstance, int imageIndex)
{
	PipelineCreateInfo createInfo = assetInstance->createInfo_;
	createInfo.isOffscreen = true;
	createDescriptorSetLayout(createInfo, assetInstance->data_.offscreenDescriptorLayout_);

	VkDescriptorSetLayout& currLayout = assetInstance->data_.offscreenDescriptorLayout_;

	std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPools_[imageIndex];
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
	allocInfo.pSetLayouts = layouts.data();

	assetInstance->data_.offscreenDescriptorSet_.resize(swapChainResources_.imageCount_);
	VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, assetInstance->data_.offscreenDescriptorSet_.data()));

	int descriptorSetSize = 0;

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = assetInstance->data_.uniformData_[imageIndex].buffer_;
	bufferInfo.offset = 0;
	bufferInfo.range = assetInstance->sizeOfUniformData_;
	descriptorSetSize++;

	VkDescriptorBufferInfo globalBufferInfo{};
	globalBufferInfo.buffer = offscreenResources_.offscreenUniformBuffers_[imageIndex].buffer_;
	globalBufferInfo.offset = 0;
	globalBufferInfo.range = sizeof(offscreenResources_.globalOffscreenUniformData_);
	descriptorSetSize++;

	std::vector<VkWriteDescriptorSet> descriptorWrites{};
	descriptorWrites.resize(descriptorSetSize);
	int descriptorWriteIdx = 0;

	descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.offscreenDescriptorSet_[imageIndex];
	descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
	descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
	descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
	descriptorWrites[descriptorWriteIdx].pBufferInfo = &bufferInfo;
	descriptorWriteIdx++;

	descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.offscreenDescriptorSet_[imageIndex];
	descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
	descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
	descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
	descriptorWrites[descriptorWriteIdx].pBufferInfo = &globalBufferInfo;
	descriptorWriteIdx++;

	vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VkRenderer::recreateSwapchain()
{
	if (width_ == 0 || height_ == 0)
	{
		recreateSwapchain_ = true;
		return;
	}

	vkDeviceWaitIdle(logicalDevice_);
	cleanupSwapchain();
	createSwapchainResources();
	createDefaultRenderPass();

	VkExtent3D extent{ width_, height_, 1 };
	createDepthImageResources(depthResources_.imageResources_, extent);
	createDefaultFramebuffers();

	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(cmdPool_, 1);
	VK_CHECK(vkAllocateCommandBuffers(logicalDevice_, &cmdAllocInfo, &cmdBuffer_));

	ui_->recreateUI(instance_, physicalDevice_, logicalDevice_, graphicsQueueFamiliy_, graphicsQueue_, uiDescriptorPool_, swapChainResources_.imageCount_, swapChainResources_.imageCount_, cmdPool_, cmdBuffer_, window_->getGLFWWindow(), renderPass_);
	static_cast<VulkanDebugDrawManager*>(debugDrawManager_)->recreateDebugDrawManager(logicalDevice_, renderPass_, debugDescriptorPool_);
}

void VkRenderer::init()
{
	width_ = window_->windowInfo_.width;
	height_ = window_->windowInfo_.height;

	vkb::InstanceBuilder builder;

	bool bUseValidationLayers;

	//TODO: Validation layers are causing memory leak when reseting descirptor pool
	//https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/3948
	#ifdef NDEBUG
		bUseValidationLayers = false;
	#else
		bUseValidationLayers = true;
	#endif

	//make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Engine")
		.request_validation_layers(bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 1, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//grab the instance 
	instance_ = vkb_inst.instance;
	debugMessenger_ = vkb_inst.debug_messenger;

	glfwCreateWindowSurface(instance_, window_->getGLFWWindow(), nullptr, &surface_);

	VkPhysicalDeviceFeatures features{};
	features.fragmentStoresAndAtomics = true;//need this feature to make a writeable storage buffer in the fragment shader

	//use vkbootstrap to select a gpu. 
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(surface_)
		.set_required_features(features)
		.select()
		.value();

	//create the final vulkan device

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	logicalDevice_ = vkbDevice.device;
	physicalDevice_ = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	graphicsQueue_ = vkbDevice.get_queue(vkb::QueueType::graphics).value();

	graphicsQueueFamiliy_ = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice_;
	allocatorInfo.device = logicalDevice_;
	allocatorInfo.instance = instance_;
	vmaCreateAllocator(&allocatorInfo, &allocator_);

	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(graphicsQueueFamiliy_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(vkCreateCommandPool(logicalDevice_, &commandPoolInfo, nullptr, &cmdPool_));

	VK_CHECK(vkCreateCommandPool(logicalDevice_, &commandPoolInfo, nullptr, &uploadContext_.cmdPool_));

	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(cmdPool_, 1);

	VK_CHECK(vkAllocateCommandBuffers(logicalDevice_, &cmdAllocInfo, &cmdBuffer_));


	windowExtent_ = { width_, height_ };

	createSwapchainResources();

	createDescriptorPools();

	createDefaultRenderPass();

	VkExtent3D extent = { width_, height_, 1 };
	createDepthImageResources(depthResources_.imageResources_, extent);
	
	createDefaultFramebuffers();

	createGlobalUniformBuffers();

	createSynchronizationResources();

	createOffscreenResources();

	ui_ = new UI::UIInterface();
	debugDrawManager_ = new VulkanDebugDrawManager();
}

void VkRenderer::createOffscreenResources()
{
	VkExtent3D depthImageExtent{ SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, 1 };
	createDepthImageResources(offscreenResources_.offscreenDepthResources_.imageResources_, depthImageExtent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, true);
	createOffscreenRenderPass();
	createOffscreenFramebuffer();

	size_t uniformBufferSize = sizeof(offscreenResources_.globalOffscreenUniformData_);
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = uniformBufferSize;
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	offscreenResources_.offscreenUniformBuffers_.resize(swapChainResources_.imageCount_);
	for (int swapChainIndex = 0; swapChainIndex < swapChainResources_.imageCount_; swapChainIndex++)
	{
		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &offscreenResources_.offscreenUniformBuffers_[swapChainIndex].buffer_, &offscreenResources_.offscreenUniformBuffers_[swapChainIndex].mem_, nullptr));
	}
}

void VkRenderer::createDefaultRenderPass()
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swapChainResources_.imageFormat_;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = depthResources_.imageFormat_;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;
	
	VK_CHECK(vkCreateRenderPass(logicalDevice_, &render_pass_info, nullptr, &renderPass_));
}

void VkRenderer::createOffscreenRenderPass()
{
	//VkAttachmentDescription colorAttachment = {};
	//colorAttachment.format = VK_FORMAT_R32_SFLOAT;
	//colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	// Depth attachment
	depthAttachment.flags = 0;
	depthAttachment.format = depthResources_.imageFormat_;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//hook the depth attachment into the subpass
	subpass.colorAttachmentCount = 0;
	//subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//renderPassInfo.attachmentCount  2;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &depthAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK(vkCreateRenderPass(logicalDevice_, &renderPassInfo, nullptr, &offscreenResources_.offscreenRenderPass_));
}

void VkRenderer::createOffscreenFramebuffer()
{
	VkExtent3D extent = 
	{
		SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, 1
	};

	offscreenResources_.offscreenFrameBuffers_.resize(swapChainResources_.imageCount_);
	for (size_t imageIdx = 0; imageIdx < swapChainResources_.imageCount_; imageIdx++) 
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = offscreenResources_.offscreenRenderPass_;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &offscreenResources_.offscreenDepthResources_.imageResources_.view_;
		framebufferInfo.width = SHADOW_WIDTH_HEIGHT;
		framebufferInfo.height = SHADOW_WIDTH_HEIGHT;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDevice_, &framebufferInfo, nullptr, &offscreenResources_.offscreenFrameBuffers_[imageIdx]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VkRenderer::prepareOffscreenDirectionalLight(const DirLight& dirLight, int imageIndex)
{
	glm::vec3 lightPos = { 5.0f, 20.0f, 0.0f };
	glm::mat4 shadowProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 1000.0f);
	//glm::mat4 shadowProj = glm::perspective(glm::radians(100.0f), aspect, near, far);
	//shadowProj[1][1] *= -1;//need to flip this because GLM uses OpenGl coordinates where top left is -1,1 where as in vulkan top left is -1,-1.  Flip y scale
	glm::mat4 lightView = glm::lookAt(lightPos, lightPos + dirLight.uniformData_.direction, Transform::worldUp);

	offscreenResources_.globalOffscreenUniformData_.lightPos = lightPos;
	offscreenResources_.globalOffscreenUniformData_.pointLightSpaceMatrix = lightView;
	offscreenResources_.globalOffscreenUniformData_.projection = shadowProj;

	void* data;
	vmaMapMemory(allocator_, offscreenResources_.offscreenUniformBuffers_[imageIndex].mem_, &data);
	memcpy(data, &offscreenResources_.globalOffscreenUniformData_, sizeof(offscreenResources_.globalOffscreenUniformData_));
	vmaUnmapMemory(allocator_, offscreenResources_.offscreenUniformBuffers_[imageIndex].mem_);
}

void VkRenderer::setImageLayout(VkImage& image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	int bytesPerPixel = 4;
	VkDeviceSize imageSize = extent.width * extent.height * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VulkanBuffer stagingBuffer;
	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);
	//transition image layout
	uploadGraphicsCommand([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range{};
		range.aspectMask = imageAspect;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.layerCount = 1;

		//transition the image back into shader readable
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.subresourceRange = range;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = image;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source 
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			_ASSERT(false);
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			_ASSERT(false);
			break;
		}

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	});

	vmaDestroyBuffer(allocator_, stagingBuffer.buffer_, stagingBuffer.mem_);
}

void VkRenderer::setImageLayout(VkImage & image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, const VkImageSubresourceRange& range, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	int bytesPerPixel = 4;
	VkDeviceSize imageSize = extent.width * extent.height * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VulkanBuffer stagingBuffer;
	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);
	//transition image layout
	uploadGraphicsCommand([&](VkCommandBuffer cmd) {
		//transition the image back into shader readable
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.subresourceRange = range;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = image;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source 
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			_ASSERT(false);
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			_ASSERT(false);
			break;
		}

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	});

	vmaDestroyBuffer(allocator_, stagingBuffer.buffer_, stagingBuffer.mem_);
}

void VkRenderer::setImageLayoutNoUpload(VkCommandBuffer cmd, VkImage & image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	int bytesPerPixel = 4;
	VkDeviceSize imageSize = extent.width * extent.height * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VulkanBuffer stagingBuffer;
	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);

	//transition image layout
	VkImageSubresourceRange range{};
	range.aspectMask = imageAspect;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.layerCount = 1;

	//transition the image back into shader readable
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.subresourceRange = range;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source 
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		_ASSERT(false);
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		_ASSERT(false);
		break;
	}

	//barrier the image into the shader readable layout
	vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void VkRenderer::setImageLayoutNoUpload(VkCommandBuffer cmd, VkImage & image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkExtent3D extent, const VkImageSubresourceRange & subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	int bytesPerPixel = 4;
	VkDeviceSize imageSize = extent.width * extent.height * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VulkanBuffer stagingBuffer;
	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);

	//transition the image back into shader readable
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source 
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		_ASSERT(false);
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		_ASSERT(false);
		break;
	}

	//barrier the image into the shader readable layout
	vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

}

void VkRenderer::prepareAssetInstanceData(const std::vector<AssetInstance*>& assetInstancesToDraw, int imageIndex)
{
	for (auto object : assetInstancesToDraw)
	{
		allocateDescriptorSet(object, imageIndex);
		createPipelineNew(object);

		void *data;
		vmaMapMemory(allocator_, object->data_.uniformData_[frameNumber_%swapChainResources_.imageCount_].mem_, &data);
		memcpy(data, object->data_.ubo_, object->sizeOfUniformData_);
		vmaUnmapMemory(allocator_, object->data_.uniformData_[frameNumber_%swapChainResources_.imageCount_].mem_);
	}
}

void VkRenderer::createDepthImageResources(TextureResources& resources, const VkExtent3D& depthImageExtent, VkImageUsageFlags usageFlags, bool createSampler)
{
	_ASSERTE(usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, ("usageFlags need to have VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT"));

	VkImageCreateInfo depthImgInfo = vkinit::image_create_info(depthResources_.imageFormat_, usageFlags, depthImageExtent);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator_, &depthImgInfo, &allocInfo, &resources.image_.image_, &resources.image_.mem_, nullptr);

	VkImageViewCreateInfo imageView = vkinit::imageview_create_info(depthResources_.imageFormat_, resources.image_.image_, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageView, nullptr, &resources.view_)); //needs to be deleted

#define SHADOWMAP_FILTER VK_FILTER_LINEAR
	if (createSampler)
	{
		// Create sampler to sample from to depth attachment 
		// Used to sample in the fragment shader for shadowed rendering
		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = SHADOWMAP_FILTER;
		sampler.minFilter = SHADOWMAP_FILTER;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK(vkCreateSampler(logicalDevice_, &sampler, nullptr, &resources.sampler_));
	}
}

void VkRenderer::createDefaultFramebuffers()
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	const uint32_t swapchain_imagecount = swapChainResources_.imageCount_;
	frameBuffers_ = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) {

		std::vector<VkImageView> attachments = {
			swapChainResources_.swapChainImageViews_[i],
			depthResources_.imageResources_.view_
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass_;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = width_;
		framebufferInfo.height = height_;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(logicalDevice_, &framebufferInfo, nullptr, &frameBuffers_[i]));
	}
}

void VkRenderer::createSwapchainResources()
{
	vkb::SwapchainBuilder swapchainBuilder(physicalDevice_, logicalDevice_, surface_);
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		//.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
		.set_desired_extent(width_, height_)
		.build()
		.value();

	//store swapchain and its related images
	swapchain_ = vkbSwapchain.swapchain;
	swapChainResources_.swapChainImages_ = vkbSwapchain.get_images().value();
	swapChainResources_.swapChainImageViews_ = vkbSwapchain.get_image_views().value();
	swapChainResources_.imageCount_ = vkbSwapchain.image_count;

	swapChainResources_.imageFormat_ = vkbSwapchain.image_format;
}

void VkRenderer::createDescriptorPools()
{
	descriptorPools_.resize(swapChainResources_.imageCount_);

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};


	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * ((int)(sizeof(poolSizes)/sizeof(*(poolSizes))));
	pool_info.poolSizeCount = (uint32_t)((int)(sizeof(poolSizes)/sizeof(*(poolSizes))));
	pool_info.pPoolSizes = poolSizes;
	for (int i = 0; i < swapChainResources_.imageCount_; i++)
	{
		vkCreateDescriptorPool(logicalDevice_, &pool_info, NULL, &descriptorPools_[i]);
	}

	vkCreateDescriptorPool(logicalDevice_, &pool_info, NULL, &uiDescriptorPool_);
	vkCreateDescriptorPool(logicalDevice_, &pool_info, NULL, &debugDescriptorPool_);
}

void VkRenderer::createSynchronizationResources()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(logicalDevice_, &fenceCreateInfo, nullptr, &renderFence_));


	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();
	//create UploadContext fence
	VK_CHECK(vkCreateFence(logicalDevice_, &uploadFenceCreateInfo, nullptr, &uploadContext_.uploadFence_));

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	VK_CHECK(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &presentSemaphore_));
	VK_CHECK(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &renderSemaphore_));
}

void VkRenderer::prepareFrame()
{
	ui_->prepareFrame();
}

void VkRenderer::uploadMesh(Renderable* mesh)
{
	mesh->uploaded_;
	//======= Vertices ======
	size_t vertexBufferSize = mesh->vertices_.size() * sizeof(Vertex);

	VkBufferCreateInfo vBufferInfo{};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.pNext = nullptr;
	vBufferInfo.size = vertexBufferSize;
	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	VK_CHECK(vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &mesh->vertexBuffer_, &mesh->vertexMem_, nullptr));

	void *vertexData;
	vmaMapMemory(allocator_, mesh->vertexMem_, &vertexData);
	memcpy(vertexData, mesh->vertices_.data(), vertexBufferSize);
	vmaUnmapMemory(allocator_, mesh->vertexMem_);

	//======= Indicies ======
	size_t indexBufferSize = mesh->indices_.size() * sizeof(uint32_t);

	VkBufferCreateInfo iBufferInfo{};
	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	iBufferInfo.pNext = nullptr;
	iBufferInfo.size = indexBufferSize;
	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	VK_CHECK(vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &mesh->indexBuffer_, &mesh->indexMem_, nullptr));

	void *indexData;
	vmaMapMemory(allocator_, mesh->indexMem_, &indexData);
	memcpy(indexData, mesh->indices_.data(), indexBufferSize);
	vmaUnmapMemory(allocator_, mesh->indexMem_);
}


template<typename VertexType>
void VkRenderer::uploadStaticMeshData(std::vector<VertexType> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer)
{
	//======= Vertices ======
	size_t vertexBufferSize = verticies.size() * sizeof(VertexType);

	VkBufferCreateInfo vBufferInfo{};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.pNext = nullptr;
	vBufferInfo.size = vertexBufferSize;
	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VK_CHECK(vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &vertexBuffer->buffer_, &vertexBuffer->mem_, nullptr));

	void *vertexData;
	vmaMapMemory(allocator_, vertexBuffer->mem_, &vertexData);
	memcpy(vertexData, verticies.data(), vertexBufferSize);
	vmaUnmapMemory(allocator_, vertexBuffer->mem_);

	//======= Indicies ======
	size_t indexBufferSize = indicies.size() * sizeof(int);

	VkBufferCreateInfo iBufferInfo{};
	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	iBufferInfo.pNext = nullptr;
	iBufferInfo.size = indexBufferSize;
	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	VK_CHECK(vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &indexBuffer->buffer_, &indexBuffer->mem_, nullptr));

	void *indexData;
	vmaMapMemory(allocator_, indexBuffer->mem_, &indexData);
	memcpy(indexData, indicies.data(), indexBufferSize);
	vmaUnmapMemory(allocator_, indexBuffer->mem_);
}

template void VkRenderer::uploadStaticMeshData<Vertex>(std::vector<Vertex> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer);

template void VkRenderer::uploadStaticMeshData<NonAnimVertex>(std::vector<NonAnimVertex> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer);

template void VkRenderer::uploadStaticMeshData<DebugVertex>(std::vector<DebugVertex> verticies, std::vector<uint32_t> indicies, VulkanBuffer* vertexBuffer, VulkanBuffer* indexBuffer);

template<typename UniformDataType>
void VkRenderer::uploadDynamicData(AssetInstance* assetInstance)
{
	createUniformBuffers<UniformDataType>(assetInstance);
	//allocateDescriptorSet(assetInstance);
	//createPipelineNew(assetInstance);
}

template void VkRenderer::uploadDynamicData<MVPBoneData>(AssetInstance* assetInstance);

template void VkRenderer::uploadDynamicData<MVP>(AssetInstance* assetInstance);

void VkRenderer::uploadTextureData(Textured* texture)
{
	VkExtent3D extent = { texture->textureHeight_, texture->textureWidth_, 1};
	VkImageCreateInfo textureInfo = vkinit::image_create_info(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(allocator_, &textureInfo, &allocInfo, &texture->resources_.image_.image_, &texture->resources_.image_.mem_, nullptr);

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	int bytesPerPixel = 4;
	VkDeviceSize imageSize = texture->textureWidth_ * texture->textureHeight_ * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VulkanBuffer stagingBuffer;

	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);

	void* data;
	vmaMapMemory(allocator_, stagingBuffer.mem_, &data);
	memcpy(data, texture->pixels_.data(), imageSize);
	vmaUnmapMemory(allocator_, stagingBuffer.mem_);

	//transition image layout
	uploadGraphicsCommand([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = texture->resources_.image_.image_;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		//now copy buffer image data into the VkImage
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = extent;

		vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer_, texture->resources_.image_.image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		//transition the image back into shader readable
		VkImageMemoryBarrier toReadable = imageBarrier_toTransfer;
		toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		toReadable.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toReadable);
	});

	VkImageViewCreateInfo imageViewCreateInfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, texture->resources_.image_.image_, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageViewCreateInfo, nullptr, &texture->resources_.view_)); //needs to be deleted

	VkSamplerCreateInfo samplerCreateInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VK_CHECK(vkCreateSampler(logicalDevice_, &samplerCreateInfo, nullptr, &texture->resources_.sampler_));

	vmaDestroyBuffer(allocator_, stagingBuffer.buffer_, stagingBuffer.mem_);
}


VkShaderModule VkRenderer::createShaderModule(std::string shaderPath) 
{
	if (shaderMap_.find(shaderPath) != shaderMap_.end())
	{
		return shaderMap_[shaderPath];
	}

	std::vector<char> code = readShaderFile(shaderPath);
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;

	VK_CHECK(vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule));

	shaderMap_.insert({ shaderPath, shaderModule });

	return shaderModule;
}

void VkRenderer::draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights)
{
	vkDeviceWaitIdle(logicalDevice_);

	if (recreateSwapchain_)
	{
		recreateSwapchain_ = false;
		recreateSwapchain();
		return;
	}

	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(logicalDevice_, 1, &renderFence_, true, 1000000000));
	VK_CHECK(vkResetFences(logicalDevice_, 1, &renderFence_));

	VK_CHECK(vkResetCommandBuffer(cmdBuffer_, 0));

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult result = (vkAcquireNextImageKHR(logicalDevice_, swapchain_, 1000000000, presentSemaphore_, nullptr, &swapchainImageIndex));

	//for (int i = 0; i < pipelines_.size(); i++)
	//{
	//	PipelineResources* pipeline = pipelines_[i];
	//	delete pipeline;
	//}
	//pipelines_.clear();
	//pipelineResourcesMap_.clear();
	vkResetDescriptorPool(logicalDevice_, descriptorPools_[swapchainImageIndex], 0);

	for (int i = 0; i < assetInstancesToDelete_.size(); i++)
	{
		AssetInstance& assetInstance = assetInstancesToDelete_[i]->getComponent<AssetInstance>();
		for (int swapChainIdx = 0; swapChainIdx < assetInstance.data_.uniformData_.size(); swapChainIdx++)
		{
			vmaDestroyBuffer(allocator_, assetInstance.data_.uniformData_[swapChainIdx].buffer_, assetInstance.data_.uniformData_[swapChainIdx].mem_);
		}
	}
	pipelineAssetMap_.clear();

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmdBuffer_, &cmdBeginInfo));

	prepareAssetInstanceData(assetInstancesToDraw, swapchainImageIndex);
		
	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	
	if(Console::getBoolVar("shadows.enabled"))
	{
		prepareOffscreenDirectionalLight(dirLight, swapchainImageIndex);
		drawOffscreenDirLight(dirLight, swapchainImageIndex, assetInstancesToDraw);
	}

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(renderPass_, {width_, height_}, frameBuffers_[swapchainImageIndex]);
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer_, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (RenderSubsystemInterface* renderSubsystem : renderSubsystems_)
	{
		renderSubsystem->renderFrame(cmdBuffer_, swapchainImageIndex);
	}

	uploadGlobalUniformData(swapchainImageIndex, dirLight, pointLights);

	drawAssetInstances(cmdBuffer_, swapchainImageIndex, assetInstancesToDraw);
	static_cast<VulkanDebugDrawManager*>(debugDrawManager_)->renderFrame(cmdBuffer_, swapchainImageIndex);
	ui_->renderFrame(&cmdBuffer_);//draw UI last

	//finalize the render pass
	vkCmdEndRenderPass(cmdBuffer_);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmdBuffer_));

	VkSubmitInfo submit = vkinit::submit_info(&cmdBuffer_);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &presentSemaphore_;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderSemaphore_;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(graphicsQueue_, 1, &submit, renderFence_));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::present_info();

	presentInfo.pSwapchains = &swapchain_;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphore_;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(graphicsQueue_, &presentInfo));

	//increase the number of frames drawn
	frameNumber_++;

	for (int i = 0; i < assetInstancesToDelete_.size(); i++)
	{
		Scene::getSingleton().deleteEntity(assetInstancesToDelete_[i]);
	}
	assetInstancesToDelete_.clear();
}

void VkRenderer::drawAssetInstances(VkCommandBuffer currentCommandBuffer, int imageIndex, const std::vector<AssetInstance*>& assetInstancesToDraw)
{
	VkDeviceSize offsets[1] = { 0 };

	for (auto pipelines : pipelineAssetMap_)
	{
		PipelineResources* pipelineResources = pipelines_[pipelineResourcesMap_[pipelines.first]];
		vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipeline_);

		for (auto object : pipelines.second)
		{
			VkDescriptorSet& descriptorSet = object->data_.descriptorSets_[imageIndex];
			vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipelineLayout_, 0, 1, &descriptorSet, 0, nullptr);

			vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &object->asset_->mesh_.vertexBuffer_.buffer_, offsets);

			vkCmdBindIndexBuffer(currentCommandBuffer, object->asset_->mesh_.indexBuffer_.buffer_, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(currentCommandBuffer, static_cast<uint32_t>(object->asset_->mesh_.numInidicies_), 1, 0, 0, 0);

		}
	}
	pipelineAssetMap_.clear();
}

void VkRenderer::cleanup()
{
	delete ui_;

	for (int i = 0; i < swapChainResources_.imageCount_; i++)
	{
		vkDestroyDescriptorPool(logicalDevice_, descriptorPools_[i], nullptr);
	}
	
	vkFreeCommandBuffers(logicalDevice_, cmdPool_, 1, &cmdBuffer_);
	vkDestroyCommandPool(logicalDevice_, cmdPool_, nullptr);
	
	vkDestroySemaphore(logicalDevice_,  presentSemaphore_,nullptr);
	vkDestroySemaphore(logicalDevice_, renderSemaphore_, nullptr);
	vkDestroyFence(logicalDevice_, renderFence_, nullptr);

	for (auto imageView : swapChainResources_.swapChainImageViews_) {
		vkDestroyImageView(logicalDevice_, imageView, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice_, swapchain_, nullptr);

	vkDestroyDevice(logicalDevice_, nullptr);

	vkDestroySurfaceKHR(instance_, surface_, nullptr);

	vkDestroyInstance(instance_, nullptr);
}

void VkRenderer::createTextureResources(Renderable& mesh, Textured& texture)
{
	VkExtent3D extent = { texture.textureHeight_, texture.textureWidth_, 1};
	VkImageCreateInfo textureInfo = vkinit::image_create_info(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(allocator_, &textureInfo, &allocInfo, &mesh.textureResources_.image_.image_, &mesh.textureResources_.image_.mem_, nullptr);

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	int bytesPerPixel = 4;
	VkDeviceSize imageSize = texture.textureWidth_ * texture.textureHeight_ * bytesPerPixel;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VulkanBuffer stagingBuffer;

	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);

	void* data;
	vmaMapMemory(allocator_, stagingBuffer.mem_, &data);
	memcpy(data, texture.pixels_.data(), imageSize);
	vmaUnmapMemory(allocator_, stagingBuffer.mem_);

	//transition image layout
	uploadGraphicsCommand([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = mesh.textureResources_.image_.image_;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		//now copy buffer image data into the VkImage
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = extent;

		vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer_, mesh.textureResources_.image_.image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		//transition the image back into shader readable
		VkImageMemoryBarrier toReadable = imageBarrier_toTransfer;
		toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		toReadable.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toReadable);
	});

	VkImageViewCreateInfo imageView = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, mesh.textureResources_.image_.image_, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageView, nullptr, &mesh.textureResources_.view_)); //needs to be deleted

	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VK_CHECK(vkCreateSampler(logicalDevice_, &samplerInfo, nullptr, &mesh.textureResources_.sampler_));

	vmaDestroyBuffer(allocator_, stagingBuffer.buffer_, stagingBuffer.mem_);
}

TextureResources VkRenderer::createTextureResources(int textureWidth, int textureHeight, int numChannles, int offset, std::vector<unsigned char>& pixels, VkImageCreateInfo& textureInfo)
{
	TextureResources ret{};

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(allocator_, &textureInfo, &allocInfo, &ret.image_.image_, &ret.image_.mem_, nullptr);

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	const int bytesPerPixel = 4;
	VkDeviceSize imageSize = textureWidth * textureHeight * bytesPerPixel * textureInfo.arrayLayers;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VulkanBuffer stagingBuffer;

	vmaCreateBuffer(allocator_, &bufferInfo, &vmaInfo, &stagingBuffer.buffer_, &stagingBuffer.mem_, nullptr);

	void* data;
	vmaMapMemory(allocator_, stagingBuffer.mem_, &data);
	memcpy(data, pixels.data(), imageSize);
	vmaUnmapMemory(allocator_, stagingBuffer.mem_);

	//transition image layout
	uploadGraphicsCommand([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = textureInfo.arrayLayers;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = ret.image_.image_;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		//now copy buffer image data into the VkImage
		for (int face = 0; face < textureInfo.arrayLayers; face++)
		{
			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = offset * face;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = face;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = textureInfo.extent;
			bufferCopyRegions.push_back(copyRegion);
		}

		vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer_, ret.image_.image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

		//transition the image back into shader readable
		VkImageMemoryBarrier toReadable = imageBarrier_toTransfer;
		toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		toReadable.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toReadable);
	});

	VkImageViewCreateInfo imageView = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, ret.image_.image_, VK_IMAGE_ASPECT_COLOR_BIT, textureInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D, textureInfo.arrayLayers);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageView, nullptr, &ret.view_)); //needs to be deleted

	return ret;
}

void VkRenderer::uploadGraphicsCommand(std::function<void(VkCommandBuffer cmd)>&& func)
{
	VkCommandBufferAllocateInfo allocInfo = vkinit::command_buffer_allocate_info(uploadContext_.cmdPool_, 1);

	VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &cmd));

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	func(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::submit_info(&cmd);

	VK_CHECK(vkQueueSubmit(graphicsQueue_, 1, &submit, uploadContext_.uploadFence_));

	vkWaitForFences(logicalDevice_, 1, &uploadContext_.uploadFence_, true, 99999999999);

	vkResetFences(logicalDevice_, 1, &uploadContext_.uploadFence_);

	vkResetCommandPool(logicalDevice_, uploadContext_.cmdPool_, 0);
}
