#include <math.h>
#include <vector>
#include <fstream>
#include "NewRenderer/PipelineBuilder.h"
#include "NewRenderer/ShaderUtils.h"
#include "DebugDrawManager.h"
#include "ResourceManager.h"
#include "EngineSettings.h"

#define MAX_VERTEX_BUFFER_SIZE 6000


std::string sphereModelLoc = "models/geosphere.obj";

DebugDrawManager::DebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator) :
	allocator_(allocator)
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

	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(PushConstantInfo);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkExtent2D extent = { EngineSettings::getSingletonPtr()->windowWidth, EngineSettings::getSingletonPtr()->windowHeight };
	debugPipeline_ = PipelineBuilder::createPipeline<Vertex>(logicalDevice, renderPass, shaders, extent, VK_NULL_HANDLE, &range, true, false, true);

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

DebugDrawManager::~DebugDrawManager()
{
}

void DebugDrawManager::renderFrame(const VkCommandBuffer& currentBuffer, const glm::mat4& projection)
{
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

		VkDeviceSize offsets[1] = { 0 };

		vkCmdBindPipeline(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipeline_->pipeline_);

		vkCmdBindVertexBuffers(currentBuffer, 0, 1, &vertexBuffer_, offsets);

		vkCmdBindIndexBuffer(currentBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < drawData_.size(); i++)
		{
			glm::mat4 mvp = projection * (scene_->getCurrentCamera()->getView()) * drawData_[i].pushConstantInfo.modelMatrix;

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
	pos.y *= -1;//I HAVE NO IDEA WHY I HAVE TO DO THIS TO GET THE DEBUG TO MATCH THE COORDINATES OF EVERYTHING ELSE
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
	mesh.numIndicies = NUM_INDICIES;
	mesh.numVerticies = NUM_VERTICIES;

	size_t vertexBufferSize = NUM_VERTICIES * sizeof(Vertex);
	size_t indexBufferSize = NUM_INDICIES * sizeof(uint32_t);

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
