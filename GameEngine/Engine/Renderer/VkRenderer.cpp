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

template<> VkRenderer* Singleton<VkRenderer>::msSingleton = 0;

VkRenderer::VkRenderer(Window& window) :
	window_(window)
{}

void VkRenderer::frameBufferResizeCallback(Events::FrameBufferResizedEvent& event)
{
	width_ = event.width_;
	height_ = event.height_;
	windowExtent_ = { width_, height_ };
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

	vkDestroyImage(logicalDevice_, depthResources_.image_, nullptr);
	vkDestroyImageView(logicalDevice_, depthResources_.depthImageView_, nullptr);

	vkFreeCommandBuffers(logicalDevice_, cmdPool_, 1, &cmdBuffer_);

	//for (int i = 0; i < PipelineTypes::NUM_PIPELINE_TYPES; i++)
	//{
	//	delete pipelines_[i];
	//}

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
	//for now only support one point light so loop is useless
	for (auto pointLight : pointLights)
	{
		globalUniformData_.pointLightData = pointLight.uniformData_;
	}

	globalUniformData_.viewPos = Scene::getSingleton().getCurrentCamera().entity_->getComponent<Transform>().position_;

	void* data;
	vmaMapMemory(allocator_, globalUniformBuffer_[imageIndex].mem_, &data);
	memcpy(data, &globalUniformData_, sizeof(GlobalUniformData));
	vmaUnmapMemory(allocator_, globalUniformBuffer_[imageIndex].mem_);
}

void VkRenderer::createPipelineNew(AssetInstance* assetInstance)
{
	const PipelineCreateInfo& createInfo = assetInstance->createInfo_;
	auto iter = pipelineMap_.find(createInfo);
	if (iter != pipelineMap_.end())
	{
		assetInstance->data_.pipeline_ = iter->second;
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

	assetInstance->data_.pipeline_ = retVals;
	pipelineMap_.insert({ createInfo, retVals });
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

template<typename UniformDataType>
void VkRenderer::allocateDescriptorSet(AssetInstance* assetInstance)
{
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	int bindingIndex = 0;

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = bindingIndex++;
	uboLayoutBinding.descriptorCount = 1; //number of elements in ubo array
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	descriptorSetLayoutBindings.push_back(uboLayoutBinding);

	if (assetInstance->asset_->texture_)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = bindingIndex++;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorSetLayoutBindings.push_back(samplerLayoutBinding);
	}

	if (assetInstance->createInfo_.lightingEnabled)
	{
		VkDescriptorSetLayoutBinding uboDirLightingBinding{};
		uboDirLightingBinding.binding = bindingIndex++;
		uboDirLightingBinding.descriptorCount = 1; //number of elements in ubo array
		uboDirLightingBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboDirLightingBinding.pImmutableSamplers = nullptr;
		uboDirLightingBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorSetLayoutBindings.push_back(uboDirLightingBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	layoutInfo.pBindings = descriptorSetLayoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &assetInstance->data_.descriptorLayout_));

	VkDescriptorSetLayout currLayout = assetInstance->data_.descriptorLayout_;

	std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool_;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
	allocInfo.pSetLayouts = layouts.data();

	assetInstance->data_.descriptorSets_.resize(swapChainResources_.imageCount_);
	VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, assetInstance->data_.descriptorSets_.data()));

	for (size_t i = 0; i < swapChainResources_.imageCount_; i++) {
		int descriptorSetSize = 0;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = assetInstance->data_.uniformData_[i].buffer_;
		bufferInfo.offset = 0;
		bufferInfo.range = assetInstance->sizeOfUniformData_;
		descriptorSetSize++;

		VkDescriptorBufferInfo lightingBufferInfo{};
		lightingBufferInfo.buffer = globalUniformBuffer_[i].buffer_;
		lightingBufferInfo.offset = 0;
		lightingBufferInfo.range = sizeof(GlobalUniformData);

		VkDescriptorImageInfo imageInfo{};
		if (assetInstance->asset_->texture_)
		{
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = assetInstance->asset_->texture_->resources_.view_;
			imageInfo.sampler = assetInstance->asset_->texture_->resources_.sampler_;
			descriptorSetSize++;
		}

		if (assetInstance->createInfo_.lightingEnabled) descriptorSetSize++;

		std::vector<VkWriteDescriptorSet> descriptorWrites{};
		descriptorWrites.resize(descriptorSetSize);
		int descriptorWriteIdx = 0;

		descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[i];
		descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
		descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
		descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
		descriptorWrites[descriptorWriteIdx].pBufferInfo = &bufferInfo;
		descriptorWriteIdx++;

		if (assetInstance->asset_->texture_)
		{
			descriptorWrites[descriptorWriteIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[i];
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
			descriptorWrites[descriptorWriteIdx].dstSet = assetInstance->data_.descriptorSets_[i];
			descriptorWrites[descriptorWriteIdx].dstBinding = descriptorWriteIdx;
			descriptorWrites[descriptorWriteIdx].dstArrayElement = 0;
			descriptorWrites[descriptorWriteIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[descriptorWriteIdx].descriptorCount = 1;
			descriptorWrites[descriptorWriteIdx].pBufferInfo = &lightingBufferInfo;
			descriptorWriteIdx++;
		}

		vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
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
	createDefualtDepthResources();
	createDefaultFramebuffers();
	//initPipelines();

	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(cmdPool_, 1);
	VK_CHECK(vkAllocateCommandBuffers(logicalDevice_, &cmdAllocInfo, &cmdBuffer_));

	ui_->recreateUI(instance_, physicalDevice_, logicalDevice_, graphicsQueueFamiliy_, graphicsQueue_, descriptorPool_, swapChainResources_.imageCount_, swapChainResources_.imageCount_, cmdPool_, cmdBuffer_, window_.getGLFWWindow(), renderPass_);
	debugDrawManager_->recreateDebugDrawManager(logicalDevice_, renderPass_, descriptorPool_);

	
}

VkRenderer* VkRenderer::getSingletonPtr()
{
	return msSingleton;
}

VkRenderer& VkRenderer::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

void VkRenderer::init()
{
	width_ = window_.windowInfo_.width;
	height_ = window_.windowInfo_.height;

	vkb::InstanceBuilder builder;

	bool bUseValidationLayers;

	#ifdef NDEBUG
		bUseValidationLayers = false;
	#else
		bUseValidationLayers = true;
	#endif

	//make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 1, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//grab the instance 
	instance_ = vkb_inst.instance;
	debugMessenger_ = vkb_inst.debug_messenger;

	glfwCreateWindowSurface(instance_, window_.getGLFWWindow(), nullptr, &surface_);

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

	VkDescriptorPoolSize pool_sizes[] =
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
	pool_info.maxSets = 1000 * ((int)(sizeof(pool_sizes)/sizeof(*(pool_sizes))));
	pool_info.poolSizeCount = (uint32_t)((int)(sizeof(pool_sizes)/sizeof(*(pool_sizes))));
	pool_info.pPoolSizes = pool_sizes;
	vkCreateDescriptorPool(logicalDevice_, &pool_info, NULL, &descriptorPool_);

	windowExtent_ = { width_, height_ };

	createSwapchainResources();

	createDefaultRenderPass();

	createDefualtDepthResources();
	
	createDefaultFramebuffers();

	createGlobalUniformBuffers();

	createSynchronizationResources();

	ui_ = new UI::UIInterface(instance_, physicalDevice_, logicalDevice_, graphicsQueueFamiliy_, graphicsQueue_, descriptorPool_, swapChainResources_.imageCount_, swapChainResources_.imageCount_, cmdPool_, cmdBuffer_, window_.getGLFWWindow(), renderPass_, allocator_);
	debugDrawManager_ = new DebugDrawManager(logicalDevice_, renderPass_, allocator_, descriptorPool_);
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

void VkRenderer::createDefualtDepthResources()
{
	VkExtent3D depthImageExtent =
	{
		width_, height_, 1
	};
	VkImageCreateInfo depthImgInfo = vkinit::image_create_info(depthResources_.imageFormat_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator_, &depthImgInfo, &allocInfo, &depthResources_.image_, &depthResources_.mem_, nullptr);

	VkImageViewCreateInfo imageView = vkinit::imageview_create_info(depthResources_.imageFormat_, depthResources_.image_, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(logicalDevice_, &imageView, nullptr, &depthResources_.depthImageView_)); //needs to be deleted
}

void VkRenderer::createDefaultFramebuffers()
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	const uint32_t swapchain_imagecount = swapChainResources_.imageCount_;
	frameBuffers_ = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) {

		std::vector<VkImageView> attachments = {
			swapChainResources_.swapChainImageViews_[i],
			depthResources_.depthImageView_
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

	////enqueue the destruction of the fence
	//_mainDeletionQueue.push_function([=]() {
	//	vkDestroyFence(_device, _renderFence, nullptr);
	//	});
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	VK_CHECK(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &presentSemaphore_));
	VK_CHECK(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &renderSemaphore_));
}

void VkRenderer::prepareFrame()
{
	ui_->prepareFrame();
}

//void VkRenderer::uploadObject(Renderable* mesh)
//{
//	if (mesh->isLine_)
//	{
//		mesh->pipelineType_ = LINE_PIPELINE;
//	}
//	else
//	{
//		mesh->pipelineType_ = DEBUG_PIPELINE;
//	}
//
//	//======= Vertices ======
//	size_t vertexBufferSize = mesh->vertices_.size() * sizeof(Vertex);
//
//	VkBufferCreateInfo vBufferInfo{};
//	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	vBufferInfo.pNext = nullptr;
//	vBufferInfo.size = vertexBufferSize;
//	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//
//	VmaAllocationCreateInfo vmaInfo{};
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	VK_CHECK(vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &mesh->vertexBuffer_, &mesh->vertexMem_, nullptr));
//
//	void *vertexData;
//	vmaMapMemory(allocator_, mesh->vertexMem_, &vertexData);
//	memcpy(vertexData, mesh->vertices_.data(), vertexBufferSize);
//	vmaUnmapMemory(allocator_, mesh->vertexMem_);
//
//	//======= Indicies ======
//	size_t indexBufferSize = mesh->indices_.size() * sizeof(uint32_t);
//
//	VkBufferCreateInfo iBufferInfo{};
//	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	iBufferInfo.pNext = nullptr;
//	iBufferInfo.size = indexBufferSize;
//	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	VK_CHECK(vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &mesh->indexBuffer_, &mesh->indexMem_, nullptr));
//
//	void *indexData;
//	vmaMapMemory(allocator_, mesh->indexMem_, &indexData);
//	memcpy(indexData, mesh->indices_.data(), indexBufferSize);
//	vmaUnmapMemory(allocator_, mesh->indexMem_);
//
//	//======= Uniform ======
//	size_t uniformBufferSize = sizeof(mesh->ubo_);
//	VkBufferCreateInfo uBufferInfo{};
//	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	uBufferInfo.pNext = nullptr;
//	uBufferInfo.size = uniformBufferSize;
//	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	mesh->uniformBuffer_.resize(swapChainResources_.imageCount_);
//	mesh->uniformMem_.resize(swapChainResources_.imageCount_);
//	for(int i = 0; i < swapChainResources_.imageCount_; i++)
//		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &mesh->uniformBuffer_[i], &mesh->uniformMem_[i], nullptr));
//
//	createDescriptorSet(mesh);
//	updateUniformBuffer(*mesh);
//
//	mesh->allocator_ = allocator_;
//	mesh->logicalDevice_ = logicalDevice_;
//}

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


void VkRenderer::uploadStaticMeshData(Renderable* mesh)
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

template<typename UniformDataType>
void VkRenderer::uploadDynamicData(AssetInstance* assetInstance)
{
	createUniformBuffers<UniformDataType>(assetInstance);
	allocateDescriptorSet<UniformDataType>(assetInstance);
	createPipelineNew(assetInstance);
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

//void VkRenderer::uploadObject(Renderable* mesh, Textured* texture, bool animated)
//{
//	if (mesh->isLine_)
//		mesh->pipelineType_ = LINE_PIPELINE;
//	else if (animated)
//		mesh->pipelineType_ = ANIMATION_PIPELINE;
//
//	//======= Vertices ======
//	size_t vertexBufferSize = mesh->vertices_.size() * sizeof(Vertex);
//
//	VkBufferCreateInfo vBufferInfo{};
//	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	vBufferInfo.pNext = nullptr;
//	vBufferInfo.size = vertexBufferSize;
//	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//
//	VmaAllocationCreateInfo vmaInfo{};
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	VK_CHECK(vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &mesh->vertexBuffer_, &mesh->vertexMem_, nullptr));
//
//	void *vertexData;
//	vmaMapMemory(allocator_, mesh->vertexMem_, &vertexData);
//	memcpy(vertexData, mesh->vertices_.data(), vertexBufferSize);
//	vmaUnmapMemory(allocator_, mesh->vertexMem_);
//
//	//======= Indicies ======
//	size_t indexBufferSize = mesh->indices_.size() * sizeof(uint32_t);
//
//	VkBufferCreateInfo iBufferInfo{};
//	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	iBufferInfo.pNext = nullptr;
//	iBufferInfo.size = indexBufferSize;
//	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	VK_CHECK(vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &mesh->indexBuffer_, &mesh->indexMem_, nullptr));
//
//	void *indexData;
//	vmaMapMemory(allocator_, mesh->indexMem_, &indexData);
//	memcpy(indexData, mesh->indices_.data(), indexBufferSize);
//	vmaUnmapMemory(allocator_, mesh->indexMem_);
//
//	//======= Uniform ======
//	size_t uniformBufferSize = sizeof(mesh->ubo_);
//	VkBufferCreateInfo uBufferInfo{};
//	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	uBufferInfo.pNext = nullptr;
//	uBufferInfo.size = uniformBufferSize;
//	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//
//	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//
//	//=DELETE=
//	mesh->uniformBuffer_.resize(swapChainResources_.imageCount_);
//	mesh->uniformMem_.resize(swapChainResources_.imageCount_);
//	for(int i = 0; i < swapChainResources_.imageCount_; i++)
//		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &mesh->uniformBuffer_[i], &mesh->uniformMem_[i], nullptr));
//
//	createTextureResources(*mesh, *texture);
//	createDescriptorSet(mesh);
//	updateUniformBuffer(*mesh);
//
//	mesh->allocator_ = allocator_;
//	mesh->logicalDevice_ = logicalDevice_;
//}

void VkRenderer::createDescriptorSet(Renderable* o)
{
	switch (o->pipelineType_)
	{
		case PipelineTypes::BASIC_PIPELINE :
		{
			VkDescriptorSetLayout currLayout = descriptorLayouts_[o->pipelineType_];
			std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
			allocInfo.pSetLayouts = layouts.data();

			o->descriptorSets_.resize(swapChainResources_.imageCount_);
			VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, o->descriptorSets_.data()));
			for (size_t i = 0; i < swapChainResources_.imageCount_; i++) {
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = o->uniformBuffer_[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(o->ubo_);

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = o->textureResources_.view_;
				imageInfo.sampler = o->textureResources_.sampler_;

				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = o->descriptorSets_[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;


				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = o->descriptorSets_[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			break;
		}

		case PipelineTypes::LINE_PIPELINE:
		{
			VkDescriptorSetLayout currLayout = descriptorLayouts_[o->pipelineType_];
			std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
			allocInfo.pSetLayouts = layouts.data();

			o->descriptorSets_.resize(swapChainResources_.imageCount_);
			VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, o->descriptorSets_.data()));
			for (size_t i = 0; i < swapChainResources_.imageCount_; i++) {
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = o->uniformBuffer_[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(o->ubo_);

				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = o->descriptorSets_[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			break;
		}

		case PipelineTypes::DEBUG_PIPELINE :
		{
			VkDescriptorSetLayout currLayout = descriptorLayouts_[o->pipelineType_];
			std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
			allocInfo.pSetLayouts = layouts.data();

			o->descriptorSets_.resize(swapChainResources_.imageCount_);
			VK_CHECK(vkAllocateDescriptorSets(logicalDevice_, &allocInfo, o->descriptorSets_.data()));
			for (size_t i = 0; i < swapChainResources_.imageCount_; i++) 
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = o->uniformBuffer_[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(o->ubo_);

				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = o->descriptorSets_[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			break;
		}
		case PipelineTypes::ANIMATION_PIPELINE:
		{
			VkDescriptorSetLayout currLayout = descriptorLayouts_[o->pipelineType_];
			std::vector<VkDescriptorSetLayout> layouts(swapChainResources_.imageCount_, currLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainResources_.imageCount_);
			allocInfo.pSetLayouts = layouts.data();

			o->descriptorSets_.resize(swapChainResources_.imageCount_);
			if (vkAllocateDescriptorSets(logicalDevice_ , &allocInfo, o->descriptorSets_.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < swapChainResources_.imageCount_; i++) 
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = o->uniformBuffer_[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(o->ubo_);

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = o->textureResources_.view_;
				imageInfo.sampler = o->textureResources_.sampler_;

				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = o->descriptorSets_[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = o->descriptorSets_[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			break;
		}
	}
}

std::vector<char> VkRenderer::readShaderFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open() || !file.good()) {
		std::cout << "ERROR: could not find shader file" << std::endl;
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
};

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

//void VkRenderer::updateUniformBuffer(Renderable& renderable)
//{
//	//renderable.ubo_.proj = glm::perspective(glm::radians(45.0f), windowExtent_.width / (float)windowExtent_.height, 0.1f, 100.0f);
//	//renderable.ubo_.proj[1][1] *= -1;
//
//	void *data;
//	size_t y = sizeof(Ubo);
//	vmaMapMemory(allocator_, renderable.uniformMem_[frameNumber_%swapChainResources_.imageCount_], &data);
//	memcpy(data, &renderable.ubo_, sizeof(Ubo));
//	vmaUnmapMemory(allocator_, renderable.uniformMem_[frameNumber_%swapChainResources_.imageCount_]);
//}

void VkRenderer::draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights)
{
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


	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmdBuffer_, &cmdBeginInfo));

	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(renderPass_, {width_, height_}, frameBuffers_[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	rpInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(cmdBuffer_, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (RenderSubsystemInterface* renderSubsystem : renderSubsystems_)
	{
		renderSubsystem->renderFrame(cmdBuffer_, swapchainImageIndex);
	}

	uploadGlobalUniformData(swapchainImageIndex, dirLight, pointLights);

	drawAssetInstances(cmdBuffer_, swapchainImageIndex, assetInstancesToDraw);
	debugDrawManager_->renderFrame(cmdBuffer_, swapchainImageIndex);
	ui_->renderFrame(cmdBuffer_);//draw UI last

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
}

void VkRenderer::drawAssetInstances(VkCommandBuffer currentCommandBuffer, int imageIndex, const std::vector<AssetInstance*>& assetInstancesToDraw)
{
	VkDeviceSize offsets[1] = { 0 };
	for (auto object : assetInstancesToDraw)
	{
		PipelineResources* pipelineResources = object->data_.pipeline_;
		vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipeline_);

		vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &object->asset_->mesh_.vertexBuffer_.buffer_, offsets);

		vkCmdBindIndexBuffer(currentCommandBuffer, object->asset_->mesh_.indexBuffer_.buffer_, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineResources->pipelineLayout_, 0, 1, &object->data_.descriptorSets_[imageIndex], 0, nullptr);

		vkCmdDrawIndexed(currentCommandBuffer, static_cast<uint32_t>(object->asset_->mesh_.numInidicies_), 1, 0, 0, 0);

		void *data;
		vmaMapMemory(allocator_, object->data_.uniformData_[frameNumber_%swapChainResources_.imageCount_].mem_, &data);
		memcpy(data, object->data_.ubo_, object->sizeOfUniformData_);
		vmaUnmapMemory(allocator_, object->data_.uniformData_[frameNumber_%swapChainResources_.imageCount_].mem_);
	}
}

void VkRenderer::cleanup()
{
	delete ui_;

	vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
	
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
