#include <iostream>
#include <fstream>
#include <vector>
#include "VkRenderer.h"
#include "PipelineBuilder.h"
#include "../libs/vk-bootstrap/VkBootstrap.h"
#include "vkinit.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../DebugDrawManager.h"
#define VMA_IMPLEMENTATION
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "GLFW/glfw3.h"


#define VK_CHECK(x) \
	do \
	{	\
		VkResult err = x;	\
		if (err)	\
		{	\
			std::cout << "ERROR: Vulkan error : " << err << std::endl;	\
			abort();	\
		} 	\
	} while(0)

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

	for (int i = 0; i < PipelineTypes::NUM_PIPELINE_TYPES; i++)
	{
		delete pipelines_[i];
	}

	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

	vkDestroySwapchainKHR(logicalDevice_, swapchain_, nullptr);

	//delete ui_;
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
	initPipelines();

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

	initPipelines();

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

void VkRenderer::uploadObject(Renderable* mesh)
{
	if (mesh->isLine_)
	{
		mesh->pipelineType_ = LINE_PIPELINE;
	}
	else
	{
		mesh->pipelineType_ = DEBUG_PIPELINE;
	}

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

	//======= Uniform ======
	size_t uniformBufferSize = sizeof(mesh->ubo_);
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = uniformBufferSize;
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	mesh->uniformBuffer_.resize(swapChainResources_.imageCount_);
	mesh->uniformMem_.resize(swapChainResources_.imageCount_);
	for(int i = 0; i < swapChainResources_.imageCount_; i++)
		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &mesh->uniformBuffer_[i], &mesh->uniformMem_[i], nullptr));

	createDescriptorSet(mesh);
	updateUniformBuffer(*mesh);

	mesh->allocator_ = allocator_;
	mesh->logicalDevice_ = logicalDevice_;
}

void VkRenderer::uploadObject(Renderable* mesh, Textured* texture, bool animated)
{
	if (mesh->isLine_)
		mesh->pipelineType_ = LINE_PIPELINE;
	else if (animated)
		mesh->pipelineType_ = ANIMATION_PIPELINE;

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

	//======= Uniform ======
	size_t uniformBufferSize = sizeof(mesh->ubo_);
	VkBufferCreateInfo uBufferInfo{};
	uBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uBufferInfo.pNext = nullptr;
	uBufferInfo.size = uniformBufferSize;
	uBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//=DELETE=
	mesh->uniformBuffer_.resize(swapChainResources_.imageCount_);
	mesh->uniformMem_.resize(swapChainResources_.imageCount_);
	for(int i = 0; i < swapChainResources_.imageCount_; i++)
		VK_CHECK(vmaCreateBuffer(allocator_, &uBufferInfo, &vmaInfo, &mesh->uniformBuffer_[i], &mesh->uniformMem_[i], nullptr));

	createTextureResources(*mesh, *texture);
	
	createDescriptorSet(mesh);
	updateUniformBuffer(*mesh);

	mesh->allocator_ = allocator_;
	mesh->logicalDevice_ = logicalDevice_;
}

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

std::vector<char> VkRenderer::readShaderFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
};

VkShaderModule VkRenderer::createShaderModule(const std::vector<char>& code) 
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VkRenderer::initPipelines()
{
	pipelines_.resize(PipelineTypes::NUM_PIPELINE_TYPES);
	descriptorLayouts_.resize(PipelineTypes::NUM_PIPELINE_TYPES);
	// BASIC_PIPELINE
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

		VkDescriptorSetLayout descriptorLayout;
		VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorLayout));

		descriptorLayouts_[PipelineTypes::BASIC_PIPELINE] = descriptorLayout;

		std::vector<char> vertexShaderCode = readShaderFile("./shaders/texturedMeshVert.spv");
		std::vector<char> fragmentShaderCode = readShaderFile("./shaders/texturedMeshFrag.spv");
		
		VkShaderModule vertexShader = createShaderModule(vertexShaderCode);
		VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";

		std::vector<VkPipelineShaderStageCreateInfo> shaders = { vertShaderStageInfo, fragShaderStageInfo };

		PipelineBuilder::PipelineResources* r = PipelineBuilder::createPipeline<Vertex>(logicalDevice_, renderPass_, shaders, windowExtent_, &descriptorLayout, std::vector<VkPushConstantRange>(), true, true);

		pipelines_[PipelineTypes::BASIC_PIPELINE] = r;
	}

	//LINE_PIPELINE
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

		VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorLayouts_[PipelineTypes::LINE_PIPELINE]));

		std::vector<char> vertexShaderCode = readShaderFile("./shaders/vert.spv");
		std::vector<char> fragmentShaderCode = readShaderFile("./shaders/frag.spv");

		VkShaderModule vertexShader = createShaderModule(vertexShaderCode);
		VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";

		std::vector<VkPipelineShaderStageCreateInfo> shaders = { vertShaderStageInfo, fragShaderStageInfo };

		PipelineBuilder::PipelineResources* r = PipelineBuilder::createPipeline<Vertex>(logicalDevice_, renderPass_, shaders, windowExtent_, &descriptorLayouts_[PipelineTypes::LINE_PIPELINE],  std::vector<VkPushConstantRange>(),true, false, true);

		pipelines_[PipelineTypes::LINE_PIPELINE] = r;
	}

	//Debug pipeline
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

		VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorLayouts_[PipelineTypes::DEBUG_PIPELINE]));

		std::vector<char> vertexShaderCode = readShaderFile("./shaders/vert.spv");
		std::vector<char> fragmentShaderCode = readShaderFile("./shaders/frag.spv");

		VkShaderModule vertexShader = createShaderModule(vertexShaderCode);
		VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";

		std::vector<VkPipelineShaderStageCreateInfo> shaders = { vertShaderStageInfo, fragShaderStageInfo };

		PipelineBuilder::PipelineResources* r = PipelineBuilder::createPipeline<Vertex>(logicalDevice_, renderPass_, shaders, windowExtent_, &descriptorLayouts_[PipelineTypes::DEBUG_PIPELINE],  std::vector<VkPushConstantRange>(), true, true);

		pipelines_[PipelineTypes::DEBUG_PIPELINE] = r;
	}

	//Animation pipeline
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

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorLayouts_[PipelineTypes::ANIMATION_PIPELINE]));

		std::vector<char> vertexShaderCode = readShaderFile("./shaders/animatedMesh.vert.spv");
		std::vector<char> fragmentShaderCode = readShaderFile("./shaders/animatedMesh.frag.spv");

		VkShaderModule vertexShader = createShaderModule(vertexShaderCode);
		VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";

		std::vector<VkPipelineShaderStageCreateInfo> shaders = { vertShaderStageInfo, fragShaderStageInfo };

		PipelineBuilder::PipelineResources* r = PipelineBuilder::createPipeline<Vertex>(logicalDevice_, renderPass_, shaders, windowExtent_, &descriptorLayouts_[PipelineTypes::ANIMATION_PIPELINE],  std::vector<VkPushConstantRange>(), true, false);

		pipelines_[PipelineTypes::ANIMATION_PIPELINE] = r;
	}
}

//void VkRenderer::updateUniformBuffer(RenderableObject& o)
void VkRenderer::updateUniformBuffer(Renderable& renderable)
{
	//renderable.ubo_.proj = glm::perspective(glm::radians(45.0f), windowExtent_.width / (float)windowExtent_.height, 0.1f, 100.0f);
	//renderable.ubo_.proj[1][1] *= -1;

	void *data;
	size_t y = sizeof(Ubo);
	vmaMapMemory(allocator_, renderable.uniformMem_[frameNumber_%swapChainResources_.imageCount_], &data);
	memcpy(data, &renderable.ubo_, sizeof(Ubo));
	vmaUnmapMemory(allocator_, renderable.uniformMem_[frameNumber_%swapChainResources_.imageCount_]);
}

void VkRenderer::drawObjects(VkCommandBuffer currentCommandBuffer, int imageIndex, std::vector<Renderable*>& objectsToDraw)
{
	VkDeviceSize offsets[1] = { 0 };
	for (auto object : objectsToDraw)
	{
		//for (RenderableObject* mesh : pipeline.second)
		//{
			vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[object->pipelineType_]->pipeline_);

			vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &object->vertexBuffer_, offsets);

			vkCmdBindIndexBuffer(currentCommandBuffer, object->indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[object->pipelineType_]->pipelineLayout_, 0, 1, &object->descriptorSets_[imageIndex], 0, nullptr);

			vkCmdDrawIndexed(currentCommandBuffer, static_cast<uint32_t>(object->indices_.size()), 1, 0, 0, 0);

			updateUniformBuffer(*object);
		//}
	}
}

void VkRenderer::draw(std::vector<Renderable*>& objectsToDraw)
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


	drawObjects(cmdBuffer_, 0, objectsToDraw);
	debugDrawManager_->renderFrame(cmdBuffer_, swapchainImageIndex);
	ui_->renderFrame(cmdBuffer_);//draw UI last
	
	for (RenderSubsystemInterface* renderSubsystem : renderSubsystems_)
		renderSubsystem->renderFrame(cmdBuffer_, swapchainImageIndex);


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
