#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "Vertex.h"
#include "BaseCamera.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "Scene/Scene.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

struct PushConstantInfo
{
	glm::mat4 modelMatrix;
};

struct PushConstantPickerVertexInfo
{
	glm::mat4 modelMatrix;
};

struct PushConstantPickerFragmentInfo
{
	alignas(4)unsigned int uniqueId;
	alignas(8)glm::vec2 mousePos;
};

struct PickerRenderInfo
{
	PushConstantPickerVertexInfo pushConstantVertexInfo;
	PushConstantPickerFragmentInfo pushConstantFragmentInfo;
	unsigned int numVerticies;
	unsigned int numIndicies;
};

struct RenderInfo
{
	PushConstantInfo pushConstantInfo;
	unsigned int numVerticies;
	unsigned int numIndicies;
};

class ENGINE_API DebugDrawManager
{
public:
	DebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool);
	~DebugDrawManager();
	void addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled = true);
	void addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled);
	void drawRect(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, float minX, float maxX, float minY, float maxY);
	void drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color, bool depthEnabled);
	void drawFilledRect(glm::vec3 pos, const glm::vec3& color,  glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, const int uniqueId);
	void renderFrame(const VkCommandBuffer& currentBuffer, const int currentImageIndex);
	bool getSelectedObject(int& id);

	Scene* scene_;

	glm::vec2 mouseInfo_;
private:

	VkDescriptorSetLayout descriptorLayout_;

	VmaAllocator& allocator_;

	VkBuffer vertexBuffer_;
	VkBuffer indexBuffer_;
	VmaAllocation vertexBufferMem_;
	VmaAllocation indexBufferMem_;

	struct {
		std::vector<PickerRenderInfo> drawData_;
		std::vector<VkDeviceSize> vertexOffsets_;
		std::vector<VkDeviceSize> indexOffsets_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indicies_;
		VkDeviceSize globalVertexOffset_ = 0;
		VkDeviceSize globalIndexOffset_ = 0;

		VkBuffer vertexBuffer_;
		VkBuffer indexBuffer_;
		std::vector<VkBuffer> storageBuffers_;
		VmaAllocation vertexBufferMem_;
		VmaAllocation indexBufferMem_;
		std::vector<VmaAllocation> storageBuffersMem_;
		VkDescriptorSetLayout storageBufferDescriptorLayout_;
		std::vector<VkDescriptorSet> descriptorSets_;

		PipelineBuilder::PipelineResources* pipeline_;
	} pickerPipelineInfo;

	std::vector<RenderInfo> drawData_;
	std::vector<VkDeviceSize> vertexOffsets_;
	std::vector<VkDeviceSize> indexOffsets_;
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indicies_;
	VkDeviceSize globalVertexOffset_ = 0;
	VkDeviceSize globalIndexOffset_ = 0;

	PipelineBuilder::PipelineResources* debugPipeline_;
};
