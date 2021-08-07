#include "UIInterface.h"
#include "libs/imgui/implot/implot.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_vulkan.h"
#include "EngineSettings.h"
#include <fstream>


template<> UI::UIInterface* Singleton<UI::UIInterface>::msSingleton = 0;

UI::UIInterface* UI::UIInterface::getSingletonPtr()
{
	return msSingleton;
}

UI::UIInterface& UI::UIInterface::getSingleton()

{
	assert(msSingleton); return (*msSingleton);
}

std::vector<char> readShaderFile(const std::string& filename) {
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

VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice) 
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

UI::UIInterface::UIInterface(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer,  GLFWwindow* window, VkRenderPass& renderPass, VmaAllocator& allocator) :
	allocator_(allocator)
{
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui_ImplGlfw_InitForVulkan(window, false);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = logicalDevice;
    init_info.QueueFamily = queueFamily;
	init_info.Queue = queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.Allocator = NULL;
	init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imageCount;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

    ImGui_ImplVulkan_Init(&init_info, renderPass);

	auto err = vkResetCommandPool(logicalDevice, commandPool, 0);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &commandBuffer;
	err = vkEndCommandBuffer(commandBuffer);
	err = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);

	vkDeviceWaitIdle(logicalDevice);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	//VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	//samplerLayoutBinding.binding = 0;
	//samplerLayoutBinding.descriptorCount = 1;
	//samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//samplerLayoutBinding.pImmutableSamplers = nullptr;
	//samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding};
	//VkDescriptorSetLayoutCreateInfo layoutInfo{};
	//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	//layoutInfo.pBindings = bindings.data();

	//vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorLayout_);

	std::vector<char> vertexShaderCode = readShaderFile("./shaders/gui2d.vert.spv");
	std::vector<char> fragmentShaderCode = readShaderFile("./shaders/gui2d.frag.spv");

	VkShaderModule vertexShader = createShaderModule(vertexShaderCode, logicalDevice);
	VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode, logicalDevice);

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

	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(PushConstantInfo);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };
	PipelineBuilder::PipelineResources* r = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, &range, true, false);

	uiPipeline_ = r;

	size_t vertexBufferSize = 2000 * 4 * sizeof(Vertex);

	VkBufferCreateInfo vBufferInfo{};
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.pNext = nullptr;
	vBufferInfo.size = vertexBufferSize;
	vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaInfo{};
	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &vertexBuffer_, &vertexBufferMem_, nullptr);

	size_t indexBufferSize = 2000 * 6 * sizeof(uint32_t);

	VkBufferCreateInfo iBufferInfo{};
	iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	iBufferInfo.pNext = nullptr;
	iBufferInfo.size = indexBufferSize;
	iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &indexBuffer_, &indexBufferMem_, nullptr);
}

void UI::UIInterface::recreateUI(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer,  GLFWwindow* window, VkRenderPass& renderPass)
{
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui_ImplGlfw_InitForVulkan(window, false);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = logicalDevice;
    init_info.QueueFamily = queueFamily;
	init_info.Queue = queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.Allocator = NULL;
	init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imageCount;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

    ImGui_ImplVulkan_Init(&init_info, renderPass);

	auto err = vkResetCommandPool(logicalDevice, commandPool, 0);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &commandBuffer;
	err = vkEndCommandBuffer(commandBuffer);
	err = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);

	vkDeviceWaitIdle(logicalDevice);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	//VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	//samplerLayoutBinding.binding = 0;
	//samplerLayoutBinding.descriptorCount = 1;
	//samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//samplerLayoutBinding.pImmutableSamplers = nullptr;
	//samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding};
	//VkDescriptorSetLayoutCreateInfo layoutInfo{};
	//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	//layoutInfo.pBindings = bindings.data();

	//vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorLayout_);

	std::vector<char> vertexShaderCode = readShaderFile("./shaders/gui2d.vert.spv");
	std::vector<char> fragmentShaderCode = readShaderFile("./shaders/gui2d.frag.spv");

	VkShaderModule vertexShader = createShaderModule(vertexShaderCode, logicalDevice);
	VkShaderModule fragmentShader = createShaderModule(fragmentShaderCode, logicalDevice);

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

	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(PushConstantInfo);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };

	delete uiPipeline_;

	uiPipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, &range, true, false);

	size_t vertexBufferSize = 4 * sizeof(Vertex);
}

void UI::UIInterface::prepareFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::UIInterface::renderFrame(VkCommandBuffer& currentBuffer)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, currentBuffer);

	size_t vertexBufferSize = vertices_.size() * sizeof(Vertex);
	size_t indexBufferSize = indicies_.size()  * sizeof(uint32_t);

	if (vertexBufferSize > 0)
	{
		void* data;
		vmaMapMemory(allocator_, vertexBufferMem_, &data);
		memcpy(data, vertices_.data(), vertexBufferSize);
		vmaUnmapMemory(allocator_, vertexBufferMem_);

		void* indexData;
		vmaMapMemory(allocator_,  indexBufferMem_, &indexData);
		memcpy(indexData, indicies_.data(), indexBufferSize);
		vmaUnmapMemory(allocator_, indexBufferMem_);

		VkDeviceSize offsets[1] = { 0 };

		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < drawData_.size(); i++)
		{

			vkCmdPushConstants(currentBuffer, uiPipeline_->pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantInfo), &drawData_[i].pushConstantInfo);

			vkCmdDrawIndexed(currentBuffer, static_cast<uint32_t>(drawData_[i].indicies.size()), 1, indexOffsets_[0], vertexOffsets_[i], 0);
		
		}
		vertices_.clear();
		indicies_.clear();
		indexOffsets_.clear();
		vertexOffsets_.clear();
		drawData_.clear();
		globalVertexOffset_ = 0;
		globalIndexOffset_ = 0;
	}
}

void UI::UIInterface::drawRect(int width, int height, glm::vec2 pos, glm::vec4 color)
{

	glm::vec2 p1 = {-width / 2, height / 2};
	glm::vec2 p2 = { width / 2, height / 2 };
	glm::vec2 p3 = { -width / 2, -height / 2};
	glm::vec2 p4 = { width / 2, -height / 2 };

	std::vector<uint32_t> indicies =
	{
		0, 1, 2, 1, 3, 2
	};

	RenderInfo mesh;
	mesh.vertices = {
		{p1, {0,0}, color},
		{p2, {1,0}, color},
		{p3, {0,1}, color},
		{p4, {1,1}, color}
	};
	mesh.indicies = indicies;

	for (int i = 0; i < indicies.size(); i++)
	{
		indicies_.push_back(indicies[i]);
	}
	for (int i = 0; i < mesh.vertices.size(); i++)
	{
		vertices_.push_back(mesh.vertices[i]);
	}

	size_t vertexBufferSize = mesh.vertices.size() * sizeof(Vertex);
	size_t indexBufferSize = mesh.indicies.size()  * sizeof(uint32_t);

	mesh.pushConstantInfo.scale = { 2.0f/EngineSettings::getSingleton().windowWidth, 2.0f/EngineSettings::getSingleton().windowHeight };

	mesh.pushConstantInfo.pos.x = -1.0f - pos.x * mesh.pushConstantInfo.scale[0];
	mesh.pushConstantInfo.pos.y = -1.0f - pos.y * mesh.pushConstantInfo.scale[1];

	drawData_.push_back(mesh);
	vertexOffsets_.push_back(globalVertexOffset_);
	indexOffsets_.push_back(globalIndexOffset_);

	//globalVertexOffset_ += vertexBufferSize-1;
	globalVertexOffset_ += mesh.vertices.size();
	//globalIndexOffset_ += indexBufferSize-1;
	globalIndexOffset_ += indicies.size();
}

UI::UIInterface::~UIInterface()
{
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}
