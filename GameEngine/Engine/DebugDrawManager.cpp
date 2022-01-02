#include <math.h>
#include <vector>
#include <fstream>
#include "Renderer/PipelineBuilder.h"
#include "Renderer/ShaderUtils.h"
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

template<> DebugDrawManager* Singleton<DebugDrawManager>::msSingleton = 0;

DebugDrawManager& DebugDrawManager::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

DebugDrawManager* DebugDrawManager::getSingletonPtr()
{
	assert(msSingleton); return msSingleton;

}

DebugDrawManager::DebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool) :
	allocator_(allocator),
	scene_(Scene::getSingletonPtr())
{
	msSingleton = this;
	//initalize vulkan pipelines
	//create normal debug pipeline
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
		debugPipelineInfo.pipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, ranges, true, false, true);

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

		vmaCreateBuffer(allocator_, &vBufferInfo, &vmaInfo, &debugPipelineInfo.vertexBuffer_, &debugPipelineInfo.vertexBufferMem_, nullptr);

		size_t indexBufferSize = 2000 * 6 * sizeof(uint32_t);

		VkBufferCreateInfo iBufferInfo{};
		iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		iBufferInfo.pNext = nullptr;
		iBufferInfo.size = indexBufferSize;
		iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		vmaInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(allocator_, &iBufferInfo, &vmaInfo, &debugPipelineInfo.indexBuffer_, &debugPipelineInfo.indexBufferMem_, nullptr);
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

		VkDescriptorSetAllocateInfo storageBufferAlloc{};
		storageBufferAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		storageBufferAlloc.descriptorPool = descriptorPool;
		storageBufferAlloc.descriptorSetCount = 1;
		storageBufferAlloc.pSetLayouts = &pickerPipelineInfo.storageBufferDescriptorLayout_;

		VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &storageBufferAlloc, &pickerPipelineInfo.descriptorSet_));

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
		storageBufferDescriptorWrite.dstSet = pickerPipelineInfo.descriptorSet_;
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
	vmaDestroyBuffer(allocator_, debugPipelineInfo.vertexBuffer_, debugPipelineInfo.vertexBufferMem_);
	vmaDestroyBuffer(allocator_, debugPipelineInfo.indexBuffer_, debugPipelineInfo.indexBufferMem_);
	vmaDestroyBuffer(allocator_, pickerPipelineInfo.vertexBuffer_, pickerPipelineInfo.vertexBufferMem_);
	vmaDestroyBuffer(allocator_, pickerPipelineInfo.indexBuffer_, pickerPipelineInfo.indexBufferMem_);
	vmaDestroyBuffer(allocator_, pickerPipelineInfo.storageBuffer_, pickerPipelineInfo.storageBufferMem_);
}

void DebugDrawManager::recreateDebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VkDescriptorPool& descriptorPool)
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
		debugPipelineInfo.pipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, ranges, true, false, true);

		vkDestroyShaderModule(logicalDevice, vertexShader, VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice, fragmentShader, VK_NULL_HANDLE);
	}
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

	}
}

void DebugDrawManager::renderFrame(const VkCommandBuffer& currentBuffer, const int currentImageIndex)
{
	VkDeviceSize offsets[1] = { 0 };
	if (debugPipelineInfo.vertices_.size() > 0)//make sure there is something to draw for this pipeline
	{
		//Calculate the total size of the debugPipeline vertex and index buffer
		size_t vertexBufferSize = debugPipelineInfo.vertices_.size() * sizeof(Vertex);
		size_t indexBufferSize = debugPipelineInfo.indicies_.size()  * sizeof(uint32_t);

		//populate the vertex buffer 
		void* data;
		vmaMapMemory(allocator_, debugPipelineInfo.vertexBufferMem_, &data);
		memcpy(data, debugPipelineInfo.vertices_.data(), vertexBufferSize);
		vmaUnmapMemory(allocator_, debugPipelineInfo.vertexBufferMem_);

		//populate the index buffer
		void* indexData;
		vmaMapMemory(allocator_,  debugPipelineInfo.indexBufferMem_, &indexData);
		memcpy(indexData, debugPipelineInfo.indicies_.data(), indexBufferSize);
		vmaUnmapMemory(allocator_, debugPipelineInfo.indexBufferMem_);

		//bind the debugPipeline and begin drawing
		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipelineInfo.pipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &debugPipelineInfo.vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, debugPipelineInfo.indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < debugPipelineInfo.drawData_.size(); i++)
		{
			// calculate the model view projection matrix using the  camera being used in the current scene_
			glm::mat4 view, proj;
			scene_->calculateViewProjection(view, proj);
			glm::mat4 mvp = proj * view * debugPipelineInfo.drawData_[i].pushConstantInfo.modelMatrix;

			vkCmdPushConstants(currentBuffer, debugPipelineInfo.pipeline_->pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantInfo), &mvp);

			vkCmdDrawIndexed(currentBuffer, static_cast<uint32_t>(debugPipelineInfo.drawData_[i].numIndicies), 1, debugPipelineInfo.indexOffsets_[i], debugPipelineInfo.vertexOffsets_[i], 0);
		
		}

		//clear all data for next frame
		debugPipelineInfo.vertices_.clear();
		debugPipelineInfo.indicies_.clear();
		debugPipelineInfo.indexOffsets_.clear();
		debugPipelineInfo.vertexOffsets_.clear();
		debugPipelineInfo.drawData_.clear();
		debugPipelineInfo.globalVertexOffset_ = 0;
		debugPipelineInfo.globalIndexOffset_ = 0;

	}
	if (pickerPipelineInfo.vertices_.size() > 0)//make sure there is something to draw for this pipeline
	{
		//Calculate the total size of the debugPipeline vertex and index buffer
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

		//bind the pickerPipeline and begin drawing
		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pickerPipelineInfo.pipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &pickerPipelineInfo.vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, pickerPipelineInfo.indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
		for (int i = 0; i < pickerPipelineInfo.drawData_.size(); i++)
		{
			// calculate the model view projection matrix using the  camera being used in the current scene_
			glm::mat4 view, proj;
			scene_->calculateViewProjection(view, proj);
			glm::mat4 mvp = proj * view * pickerPipelineInfo.drawData_[i].pushConstantVertexInfo.modelMatrix;

			//this pipeline needs to push info to both the vertex and fragment shader
			vkCmdPushConstants(currentBuffer, pickerPipelineInfo.pipeline_->pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantPickerVertexInfo), &mvp);

			pickerPipelineInfo.drawData_[i].pushConstantFragmentInfo.mousePos = mouseInfo_;
			
			//pass in the current mouse data and unqiue id for this mesh into the fragment shader for mouse picking
			vkCmdPushConstants(currentBuffer, pickerPipelineInfo.pipeline_->pipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantPickerVertexInfo), sizeof(PushConstantPickerFragmentInfo), &pickerPipelineInfo.drawData_[i].pushConstantFragmentInfo);

			vkCmdBindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pickerPipelineInfo.pipeline_->pipelineLayout_, 0, 1, &pickerPipelineInfo.descriptorSet_, 0, nullptr);

			vkCmdDrawIndexed(currentBuffer, static_cast<uint32_t>(pickerPipelineInfo.drawData_[i].numIndicies), 1, pickerPipelineInfo.indexOffsets_[i], pickerPipelineInfo.vertexOffsets_[i], 0);
		}

		//clear storage buffer every frame
		vmaMapMemory(allocator_, pickerPipelineInfo.storageBufferMem_, &data);
		unsigned int* depthArrayData = static_cast<unsigned int*>(data);
		std::memset(depthArrayData, 0, sizeof(int) * DEPTH_ARRAY_SCALE);
		vmaUnmapMemory(allocator_, pickerPipelineInfo.storageBufferMem_);

		//clear data for next frame
		pickerPipelineInfo.vertices_.clear();
		pickerPipelineInfo.indicies_.clear();
		pickerPipelineInfo.indexOffsets_.clear();
		pickerPipelineInfo.vertexOffsets_.clear();
		pickerPipelineInfo.drawData_.clear();
		pickerPipelineInfo.globalVertexOffset_ = 0;
		pickerPipelineInfo.globalIndexOffset_ = 0;
	}
}

//Create a storage buffer that will be readable by the CPU and populated by the fragement shader.
//The fragment shader will use the current depth value at the mouses current position to calculate which index in the storage buffer to store the unqiueId of the object it is currently working on.
//The uniqueId is passed in as the last argument in drawFilledRect
//the first non-zero id in the depth buffer is whatever object you are currently hovering over.
//The client can then call this method to see if the mouse is currently hovering over one of the drawFilledRect's that it created.
//Got this idea from https://blog.gavs.space/post/003-vulkan-mouse-picking/
bool DebugDrawManager::getSelectedObject(int& id)
{
	bool found = false;//was an object selected
	void* data;//storage buffer data
	vmaMapMemory(allocator_, pickerPipelineInfo.storageBufferMem_, &data);

	unsigned int* depthArrayData = static_cast<unsigned int*>(data);//cast the pointer to an array
	for (int i = 0; i < DEPTH_ARRAY_SCALE; i++)
	{
		if (depthArrayData[i] != 0)//if an element is 0 then that means a rect is being hovered over
		{
			id = depthArrayData[i];
			found = true;
		}
	}
	vmaUnmapMemory(allocator_, pickerPipelineInfo.storageBufferMem_);
	return found;
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
void DebugDrawManager::drawFilledRect(glm::vec3 pos, const glm::vec3& color,  glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, int uniqueId)
{
	const unsigned int NUM_INDICIES = 6;
	const unsigned int NUM_VERTICIES = 4;
	PickerRenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	//Need to fix this so that the X can be adjusted.  Currently assuming that we will only be looking down the X axis
	glm::vec3 p1 = { 0.0f, minX, maxY };
	glm::vec3 p2 = { 0.0f, minX, minY };
	glm::vec3 p3 = { 0.0f, maxX, minY };
	glm::vec3 p4 = { 0.0f, maxX, maxY };

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
	model = glm::translate(model, pos);

	mesh.pushConstantVertexInfo.modelMatrix = model;
	mesh.pushConstantFragmentInfo.uniqueId = uniqueId;

	pickerPipelineInfo.drawData_.push_back(mesh);
	pickerPipelineInfo.vertexOffsets_.push_back(pickerPipelineInfo.globalVertexOffset_);
	pickerPipelineInfo.indexOffsets_.push_back(pickerPipelineInfo.globalIndexOffset_);

	pickerPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	pickerPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
}

void DebugDrawManager::drawCube(glm::vec3 pos, glm::vec3 size /*L x W x H*/, glm::vec3 color)
{
	const unsigned int NUM_INDICIES = 24;
	const unsigned int NUM_VERTICIES = 8;
	RenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	glm::vec3 p0 = {size.x/2, size.y/2, size.z/2};
	glm::vec3 p1 = {size.x/2, size.y/2, -size.z/2};
	glm::vec3 p2 = {size.x/2, -size.y/2, -size.z/2};
	glm::vec3 p3 = {size.x/2, -size.y/2, size.z/2};

	glm::vec3 p4 = {-size.x/2, size.y/2, size.z/2};
	glm::vec3 p5 = {-size.x/2, size.y/2, -size.z/2};
	glm::vec3 p6 = {-size.x/2, -size.y/2, -size.z/2};
	glm::vec3 p7 = {-size.x/2, -size.y/2, size.z/2};

	debugPipelineInfo.vertices_.push_back({ p0, color });
	debugPipelineInfo.vertices_.push_back({ p1, color });
	debugPipelineInfo.vertices_.push_back({ p2, color });
	debugPipelineInfo.vertices_.push_back({ p3, color });
	debugPipelineInfo.vertices_.push_back({ p4, color });
	debugPipelineInfo.vertices_.push_back({ p5, color });
	debugPipelineInfo.vertices_.push_back({ p6, color });
	debugPipelineInfo.vertices_.push_back({ p7, color });

	debugPipelineInfo.indicies_.push_back(0);
	debugPipelineInfo.indicies_.push_back(1);
	debugPipelineInfo.indicies_.push_back(0);
	debugPipelineInfo.indicies_.push_back(4);
	debugPipelineInfo.indicies_.push_back(0);
	debugPipelineInfo.indicies_.push_back(3);

	debugPipelineInfo.indicies_.push_back(1);
	debugPipelineInfo.indicies_.push_back(5);
	debugPipelineInfo.indicies_.push_back(1);
	debugPipelineInfo.indicies_.push_back(2);

	debugPipelineInfo.indicies_.push_back(2);
	debugPipelineInfo.indicies_.push_back(6);
	debugPipelineInfo.indicies_.push_back(2);
	debugPipelineInfo.indicies_.push_back(3);

	debugPipelineInfo.indicies_.push_back(3);
	debugPipelineInfo.indicies_.push_back(7);

	debugPipelineInfo.indicies_.push_back(4);
	debugPipelineInfo.indicies_.push_back(7);
	debugPipelineInfo.indicies_.push_back(4);
	debugPipelineInfo.indicies_.push_back(5);

	debugPipelineInfo.indicies_.push_back(5);
	debugPipelineInfo.indicies_.push_back(6);

	debugPipelineInfo.indicies_.push_back(6);
	debugPipelineInfo.indicies_.push_back(7);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);

	mesh.pushConstantInfo.modelMatrix = model;

	debugPipelineInfo.drawData_.push_back(mesh);
	debugPipelineInfo.vertexOffsets_.push_back(debugPipelineInfo.globalVertexOffset_);
	debugPipelineInfo.indexOffsets_.push_back(debugPipelineInfo.globalIndexOffset_);

	debugPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	debugPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
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
void DebugDrawManager::drawRect(glm::vec3 pos, glm::vec3 color, float minX, float maxX, float minY, float maxY)
{
	const unsigned int NUM_INDICIES = 8;
	const unsigned int NUM_VERTICIES = 4;
	RenderInfo mesh;
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	//Need to fix this so that the X can be adjusted.  Currently assuming that we will only be looking down the X axis
	glm::vec3 p1 = { 0.0f, minX, maxY };
	glm::vec3 p2 = { 0.0f, maxX, maxY };
	glm::vec3 p3 = { 0.0f, minX, minY };
	glm::vec3 p4 = { 0.0f, maxX, minY };

	debugPipelineInfo.vertices_.push_back({ p1, color});
	debugPipelineInfo.vertices_.push_back({ p2, color});
	debugPipelineInfo.vertices_.push_back({ p3, color});
	debugPipelineInfo.vertices_.push_back({ p4, color});

	debugPipelineInfo.indicies_.push_back(0);
	debugPipelineInfo.indicies_.push_back(1);

	debugPipelineInfo.indicies_.push_back(1);
	debugPipelineInfo.indicies_.push_back(3);

	debugPipelineInfo.indicies_.push_back(3);
	debugPipelineInfo.indicies_.push_back(2);

	debugPipelineInfo.indicies_.push_back(2);
	debugPipelineInfo.indicies_.push_back(0);

	size_t vertexBufferSize = NUM_INDICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);
	
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);

	mesh.pushConstantInfo.modelMatrix = model;

	debugPipelineInfo.drawData_.push_back(mesh);
	debugPipelineInfo.vertexOffsets_.push_back(debugPipelineInfo.globalVertexOffset_);
	debugPipelineInfo.indexOffsets_.push_back(debugPipelineInfo.globalIndexOffset_);

	debugPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	debugPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
}

/*
p1 --------------- p2
*/
void DebugDrawManager::drawLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color)
{
	unsigned int NUM_INDICIES = 2;
	unsigned int NUM_VERTICIES = NUM_INDICIES;
	RenderInfo mesh;
	mesh.pushConstantInfo.modelMatrix = glm::mat4(1.0f);
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	size_t vertexBufferSize = NUM_VERTICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);
	
	debugPipelineInfo.vertices_.push_back({ fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } });
	debugPipelineInfo.vertices_.push_back({ toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } });

	debugPipelineInfo.indicies_.push_back(0);
	debugPipelineInfo.indicies_.push_back(1);

	debugPipelineInfo.drawData_.push_back(mesh);
	debugPipelineInfo.vertexOffsets_.push_back(debugPipelineInfo.globalVertexOffset_);
	debugPipelineInfo.indexOffsets_.push_back(debugPipelineInfo.globalIndexOffset_);
	debugPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	debugPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
}

/*
-----------------------------
| | | | | | | | | | | | | | |
-----------------------------
| | | | | | | | | | | | | | |
-----------------------------
| | | | | | | | | | | | | | |
-----------------------------
*/

void DebugDrawManager::drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color)
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
		debugPipelineInfo.vertices_.push_back({p1, color});
		debugPipelineInfo.indicies_.push_back(i * 2);
		debugPipelineInfo.vertices_.push_back({p2, color});
		debugPipelineInfo.indicies_.push_back(i * 2 + 1);

		p1.y++;
		p2.y++;
	}

	for (int i = 0; i < width; i++)
	{
		debugPipelineInfo.vertices_.push_back({p3, color});
		debugPipelineInfo.indicies_.push_back((height*2) + (i * 2));
		debugPipelineInfo.vertices_.push_back({p4, color});
		debugPipelineInfo.indicies_.push_back((height*2) + (i * 2 + 1));

		p3.x++;
		p4.x++;
	}
	
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotationAngle), axisOfRotation);

	mesh.pushConstantInfo.modelMatrix = model;

	debugPipelineInfo.drawData_.push_back(mesh);
	debugPipelineInfo.vertexOffsets_.push_back(debugPipelineInfo.globalVertexOffset_);
	debugPipelineInfo.indexOffsets_.push_back(debugPipelineInfo.globalIndexOffset_);
	debugPipelineInfo.globalVertexOffset_ += NUM_VERTICIES;
	debugPipelineInfo.globalIndexOffset_ += NUM_INDICIES;
}
