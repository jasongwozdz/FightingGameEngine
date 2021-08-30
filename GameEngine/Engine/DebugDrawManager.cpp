#include <math.h>
#include <vector>
#include <fstream>
#include "NewRenderer/PipelineBuilder.h"
#include "NewRenderer/ShaderUtils.h"
#include "DebugDrawManager.h"
#include "ResourceManager.h"
#include "EngineSettings.h"

#define MAX_VERTEX_BUFFER_SIZE 6000
#define DEPTH_ARRAY_SCALE 32//should match whats in debugPicker.frag

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

DebugDrawManager::DebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool) :
	allocator_(allocator)
{
	{
		std::vector<char> vertexShaderCode = ShaderUtils::readShaderFile("./shaders/debug.vert.spv");
		std::vector<char> fragmentShaderCode = ShaderUtils::readShaderFile("./shaders/debug.frag.spv");

		VkShaderModule vertexShader = ShaderUtils::createShaderModule(vertexShaderCode, logicalDevice);
		VkShaderModule fragmentShader = ShaderUtils::createShaderModule(fragmentShaderCode, logicalDevice);

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

		std::vector<VkPushConstantRange> ranges;
		VkPushConstantRange range;
		range.offset = 0;
		range.size = sizeof(PushConstantInfo);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ranges.push_back(range);

		VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };
		debugPipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, ranges, true, false, true);

		vkDestroyShaderModule(logicalDevice, vertexShader, VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice, fragmentShader, VK_NULL_HANDLE);

		size_t vertexBufferSize = MAX_VERTEX_BUFFER_SIZE * 4 * sizeof(Vertex);

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

	//create picker pipeline
	{

		VkDescriptorSetLayoutBinding storageBufferLayoutBinding{};
		storageBufferLayoutBinding.binding = 0;
		storageBufferLayoutBinding.descriptorCount = 1; 
		storageBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storageBufferLayoutBinding.pImmutableSamplers = nullptr;
		storageBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &storageBufferLayoutBinding;

		vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &pickerPipelineInfo.storageBufferDescriptorLayout_);

		std::vector<char> vertexShaderCode = ShaderUtils::readShaderFile("./shaders/debugPicker.vert.spv");
		std::vector<char> fragmentShaderCode = ShaderUtils::readShaderFile("./shaders/debugPicker.frag.spv");

		VkShaderModule vertexShader = ShaderUtils::createShaderModule(vertexShaderCode, logicalDevice);
		VkShaderModule fragmentShader = ShaderUtils::createShaderModule(fragmentShaderCode, logicalDevice);

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

		std::vector<VkPushConstantRange> ranges;
		VkPushConstantRange range;
		range.offset = 0;
		range.size = sizeof(PushConstantPickerVertexInfo);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ranges.push_back(range);

		range.offset = sizeof(PushConstantPickerVertexInfo);
		range.size = sizeof(PushConstantPickerFragmentInfo);
		range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		ranges.push_back(range);

		VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };
		pickerPipelineInfo.pipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, &pickerPipelineInfo.storageBufferDescriptorLayout_, ranges, true, false);

		vkDestroyShaderModule(logicalDevice, vertexShader, VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice, fragmentShader, VK_NULL_HANDLE);

		size_t vertexBufferSize = MAX_VERTEX_BUFFER_SIZE * 4 * sizeof(Vertex);

		VkBufferCreateInfo vBufferInfo{};
		vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vBufferInfo.pNext = nullptr;
		vBufferInfo.size = vertexBufferSize;
		vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VmaAllocationCreateInfo vmaInfo{};
		vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &pickerPipelineInfo.vertexBuffer_, &pickerPipelineInfo.vertexBufferMem_, nullptr);

		size_t indexBufferSize = 2000 * 6 * sizeof(uint32_t);

		VkBufferCreateInfo iBufferInfo{};
		iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		iBufferInfo.pNext = nullptr;
		iBufferInfo.size = indexBufferSize;
		iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &pickerPipelineInfo.indexBuffer_, &pickerPipelineInfo.indexBufferMem_, nullptr);

		//create storage buffer
		//need to create a buffer for eachframe
		const unsigned int imageCount = VkRenderer::getSingleton().swapChainResources_.imageCount_;
		pickerPipelineInfo.descriptorSets_.resize(imageCount);

		VkDescriptorSetAllocateInfo storageBufferAlloc{};
		storageBufferAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		storageBufferAlloc.descriptorPool = descriptorPool;
		storageBufferAlloc.descriptorSetCount = 1;
		storageBufferAlloc.pSetLayouts = &pickerPipelineInfo.storageBufferDescriptorLayout_;

		VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &storageBufferAlloc, &pickerPipelineInfo.descriptorSets_[0]));

		VkBufferCreateInfo sBufferInfo{};
		sBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		sBufferInfo.pNext = nullptr;
		sBufferInfo.size = sizeof(int) * DEPTH_ARRAY_SCALE;
		sBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		vmaInfo = {};
		vmaInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		vmaCreateBuffer(allocator_, &sBufferInfo, &vmaInfo, &pickerPipelineInfo.storageBuffer_, &pickerPipelineInfo.storageBufferMem_, nullptr);

		VkDescriptorBufferInfo storageBufferInfo{};
		storageBufferInfo.buffer = pickerPipelineInfo.storageBuffer_;
		storageBufferInfo.offset = 0;
		storageBufferInfo.range = sizeof(int) * DEPTH_ARRAY_SCALE;

		VkWriteDescriptorSet storageBufferDescriptorWrite{};
		storageBufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		storageBufferDescriptorWrite.dstSet = pickerPipelineInfo.descriptorSets_[0];
		storageBufferDescriptorWrite.dstBinding = 0;
		storageBufferDescriptorWrite.dstArrayElement = 0;
		storageBufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storageBufferDescriptorWrite.descriptorCount = 1;
		storageBufferDescriptorWrite.pBufferInfo = &storageBufferInfo;

		vkUpdateDescriptorSets(logicalDevice, 1, &storageBufferDescriptorWrite, 0, nullptr);
	}
}

DebugDrawManager::~DebugDrawManager()
{
	vmaDestroyBuffer(allocator_, vertexBuffer_, vertexBufferMem_);
	vmaDestroyBuffer(allocator_, indexBuffer_, indexBufferMem_);

	vmaDestroyBuffer(allocator_, pickerPipelineInfo.vertexBuffer_, pickerPipelineInfo.vertexBufferMem_);
	vmaDestroyBuffer(allocator_, pickerPipelineInfo.indexBuffer_, pickerPipelineInfo.indexBufferMem_);
	vmaDestroyBuffer(allocator_, pickerPipelineInfo.storageBuffer_, pickerPipelineInfo.storageBufferMem_);

}

void DebugDrawManager::renderFrame(const VkCommandBuffer& currentBuffer, const int currentImageIndex)
{

	VkDeviceSize offsets[1] = { 0 };
	if (vertices_.size() > 0)
	{
		size_t vertexBufferSize = vertices_.size() * sizeof(Vertex);
		size_t indexBufferSize = indicies_.size()  * sizeof(uint32_t);

		void* data;
		vmaMapMemory(allocator_, vertexBufferMem_, &data);
		memcpy(data, vertices_.data(), vertexBufferSize);
		vmaUnmapMemory(allocator_, vertexBufferMem_);

		void* indexData;
		vmaMapMemory(allocator_,  indexBufferMem_, &indexData);
		memcpy(indexData, indicies_.data(), indexBufferSize);
		vmaUnmapMemory(allocator_, indexBufferMem_);

		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < drawData_.size(); i++)
		{
			glm::mat4 mvp = (scene_->getCurrentCamera()->projectionMatrix) * (scene_->getCurrentCamera()->getView()) * drawData_[i].pushConstantInfo.modelMatrix;

			vkCmdPushConstants(currentBuffer, debugPipeline_->pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantInfo), &mvp);

			vkCmdDrawIndexed(currentBuffer, static_cast<uint32_t>(drawData_[i].numIndicies), 1, indexOffsets_[i], vertexOffsets_[i], 0);
		
		}

		vertices_.clear();
		indicies_.clear();
		indexOffsets_.clear();
		vertexOffsets_.clear();
		drawData_.clear();
		globalVertexOffset_ = 0;
		globalIndexOffset_ = 0;

	}
	if (pickerPipelineInfo.vertices_.size() > 0)
	{
		size_t vertexBufferSize = pickerPipelineInfo.vertices_.size() * sizeof(Vertex);
		size_t indexBufferSize = pickerPipelineInfo.indicies_.size() * sizeof(uint32_t);

		void* data;
		vmaMapMemory(allocator_, pickerPipelineInfo.vertexBufferMem_, &data);
		memcpy(data, pickerPipelineInfo.vertices_.data(), vertexBufferSize);
		vmaUnmapMemory(allocator_, pickerPipelineInfo.vertexBufferMem_);

		void* indexData;
		vmaMapMemory(allocator_, pickerPipelineInfo.indexBufferMem_, &indexData);
		memcpy(indexData, pickerPipelineInfo.indicies_.data(), indexBufferSize);
		vmaUnmapMemory(allocator_, pickerPipelineInfo.indexBufferMem_);

		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pickerPipelineInfo.pipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &pickerPipelineInfo.vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, pickerPipelineInfo.indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
		for (int i = 0; i < pickerPipelineInfo.drawData_.size(); i++)
		{
			glm::mat4 mvp = (scene_->getCurrentCamera()->projectionMatrix) * (scene_->getCurrentCamera()->getView()) * pickerPipelineInfo.drawData_[i].pushConstantVertexInfo.modelMatrix;

			vkCmdPushConstants(currentBuffer, pickerPipelineInfo.pipeline_->pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantPickerVertexInfo), &mvp);

			pickerPipelineInfo.drawData_[i].pushConstantFragmentInfo.mousePos = mouseInfo_;

			vkCmdPushConstants(currentBuffer, pickerPipelineInfo.pipeline_->pipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantPickerVertexInfo), sizeof(PushConstantPickerFragmentInfo), &pickerPipelineInfo.drawData_[i].pushConstantFragmentInfo);

			vkCmdBindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pickerPipelineInfo.pipeline_->pipelineLayout_, 0, 1, &pickerPipelineInfo.descriptorSets_[0], 0, nullptr);

			vkCmdDrawIndexed(currentBuffer, static_cast<uint32_t>(pickerPipelineInfo.drawData_[i].numIndicies), 1, pickerPipelineInfo.indexOffsets_[i], pickerPipelineInfo.vertexOffsets_[i], 0);
		}

		pickerPipelineInfo.vertices_.clear();
		pickerPipelineInfo.indicies_.clear();
		pickerPipelineInfo.indexOffsets_.clear();
		pickerPipelineInfo.vertexOffsets_.clear();
		pickerPipelineInfo.drawData_.clear();
		pickerPipelineInfo.globalVertexOffset_ = 0;
		pickerPipelineInfo.globalIndexOffset_ = 0;
	}
}

bool DebugDrawManager::getSelectedObject(int& id)
{
	bool found = false;
	void* data;
	vmaMapMemory(allocator_, pickerPipelineInfo.storageBufferMem_, &data);

	unsigned int* depthArrayData = static_cast<unsigned int*>(data);
	for (int i = 0; i < DEPTH_ARRAY_SCALE; i++)
	{
		if (depthArrayData[i] != 0)
		{
			id = depthArrayData[i];
			found = true;
		}
	}

	std::memset(depthArrayData, 0, sizeof(int)* DEPTH_ARRAY_SCALE);

	vmaUnmapMemory(allocator_, pickerPipelineInfo.storageBufferMem_);
	return found;
}

void DebugDrawManager::drawFilledRect(glm::vec3 pos, const glm::vec3& color,  glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, int uniqueId)
{
	const unsigned int NUM_INDICIES = 6;
	const unsigned int NUM_VERTICIES = 4;
	PickerRenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	glm::vec3 p1 = { 0.0f, minX, maxY };
	glm::vec3 p2 = { 0.0f, minX, minY };
	glm::vec3 p3 = { 0.0f, maxX, minY };
	glm::vec3 p4 = { 0.0f, maxX, maxY };

	//glm::vec3 p1 = { minX, 0.0f, maxY };
	//glm::vec3 p2 = { minX, 0.0f, minY };
	//glm::vec3 p3 = { maxX, 0.0f, minY };
	//glm::vec3 p4 = { maxX, 0.0f, maxY };

	pickerPipelineInfo.vertices_.push_back({ p1, color});
	pickerPipelineInfo.vertices_.push_back({ p2, color});
	pickerPipelineInfo.vertices_.push_back({ p3, color});
	pickerPipelineInfo.vertices_.push_back({ p4, color});

	pickerPipelineInfo.indicies_.push_back(0);
	pickerPipelineInfo.indicies_.push_back(1);
	pickerPipelineInfo.indicies_.push_back(2);

	pickerPipelineInfo.indicies_.push_back(0);
	pickerPipelineInfo.indicies_.push_back(2);
	pickerPipelineInfo.indicies_.push_back(3);

	glm::mat4 model = glm::mat4(1.0f);
	pos.y *= -1;//I HAVE NO IDEA WHY I HAVE TO DO THIS TO GET THE DEBUG TO MATCH THE COORDINATES OF EVERYTHING ELSE
	model = glm::translate(model, pos);

	mesh.pushConstantVertexInfo.modelMatrix = model;
	mesh.pushConstantFragmentInfo.uniqueId = uniqueId;

	pickerPipelineInfo.drawData_.push_back(mesh);
	pickerPipelineInfo.vertexOffsets_.push_back(pickerPipelineInfo.globalVertexOffset_);
	pickerPipelineInfo.indexOffsets_.push_back(pickerPipelineInfo.globalIndexOffset_);

	pickerPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	pickerPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
}
/*
 p1-------------p2
 |				 |
 |				 |
 |				 |
 |				 |
 |				 |
 p3-------------p4
*/
void DebugDrawManager::drawRect(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, float minX, float maxX, float minY, float maxY)
{
	const unsigned int NUM_INDICIES = 8;
	const unsigned int NUM_VERTICIES = 4;
	RenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	glm::vec3 p1 = { 0.0f, minX, maxY };
	glm::vec3 p2 = { 0.0f, maxX, maxY };
	glm::vec3 p3 = { 0.0f, minX, minY };
	glm::vec3 p4 = { 0.0f, maxX, minY };

	vertices_.push_back({ p1, color});
	vertices_.push_back({ p2, color});
	vertices_.push_back({ p3, color});
	vertices_.push_back({ p4, color});

	indicies_.push_back(0);
	indicies_.push_back(1);

	indicies_.push_back(1);
	indicies_.push_back(3);

	indicies_.push_back(3);
	indicies_.push_back(2);

	indicies_.push_back(2);
	indicies_.push_back(0);

	size_t vertexBufferSize = NUM_INDICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);
	
	glm::mat4 model = glm::mat4(1.0f);
	//pos.y *= -1;//I HAVE NO IDEA WHY I HAVE TO DO THIS TO GET THE DEBUG TO MATCH THE COORDINATES OF EVERYTHING ELSE
	model = glm::translate(model, pos);

	mesh.pushConstantInfo.modelMatrix = model;

	drawData_.push_back(mesh);
	vertexOffsets_.push_back(globalVertexOffset_);
	indexOffsets_.push_back(globalIndexOffset_);

	globalVertexOffset_ += NUM_VERTICIES;
	globalIndexOffset_ += NUM_INDICIES;
}

void DebugDrawManager::addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled)
{
	//ModelReturnVals	returnVals = ResourceManager::getSingleton().loadObjFile(sphereModelLoc);
	//Entity* point = scene_->addEntity("point");
	//Renderable& r = point->addComponent<Renderable>(returnVals.vertices, returnVals.indices, true, "debug_point");
	//Transform& t = point->addComponent<Transform>(pos);
	//t.setScale(5.f);
	//t.parent_ = parent;
	//return point;
}

void DebugDrawManager::addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled)
{
	unsigned int NUM_INDICIES = 2;
	unsigned int NUM_VERTICIES = NUM_INDICIES;
	RenderInfo mesh;
	mesh.pushConstantInfo.modelMatrix = glm::mat4(1.0f);
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	size_t vertexBufferSize = NUM_VERTICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);
	
	fromPos.x *= -1;//coordinates still don't match on screen
	toPos.x *= -1;

	vertices_.push_back({ fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } });
	vertices_.push_back({ toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } });
	
	indicies_.push_back(0);
	indicies_.push_back(1);

	drawData_.push_back(mesh);
	vertexOffsets_.push_back(globalVertexOffset_);
	indexOffsets_.push_back(globalIndexOffset_);
	globalVertexOffset_ += NUM_VERTICIES;
	globalIndexOffset_ += NUM_INDICIES;
}

void DebugDrawManager::drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color, bool depthEnabled)
{
	unsigned int NUM_INDICIES = (height + width) * 2;
	unsigned int NUM_VERTICIES = NUM_INDICIES;
	RenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	size_t vertexBufferSize = NUM_VERTICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);

	glm::vec3 p1 = { -width / 2, -height / 2, 0 };
	glm::vec3 p2 = { width / 2, -height / 2, 0 };

	glm::vec3 p3 = {-width / 2, -height / 2, 0};
	glm::vec3 p4 = {-width / 2, height / 2, 0};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < height; i++)
	{
		vertices_.push_back({p1, color});
		indicies_.push_back(i * 2);
		vertices_.push_back({p2, color});
		indicies_.push_back(i * 2 + 1);

		p1.y++;
		p2.y++;
	}

	for (int i = 0; i < width; i++)
	{
		vertices_.push_back({p3, color});
		indicies_.push_back((height*2) + (i * 2));
		vertices_.push_back({p4, color});
		indicies_.push_back((height*2) + (i * 2 + 1));

		p3.x++;
		p4.x++;
	}
	
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotationAngle), axisOfRotation);

	mesh.pushConstantInfo.modelMatrix = model;

	drawData_.push_back(mesh);
	vertexOffsets_.push_back(globalVertexOffset_);
	indexOffsets_.push_back(globalIndexOffset_);
	globalVertexOffset_ += NUM_VERTICIES;
	globalIndexOffset_ += NUM_INDICIES;

}
