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

struct RenderInfo
{
	PushConstantInfo pushConstantInfo;
	unsigned int numVerticies;
	unsigned int numIndicies;
};

class ENGINE_API DebugDrawManager
{
public:
	DebugDrawManager(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator);
	~DebugDrawManager();
	void addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled = true);
	void addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled);
	void drawRect(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, float minX, float maxX, float minY, float maxY);
	void drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color, bool depthEnabled);
	void renderFrame(const VkCommandBuffer& currentBuffer, const glm::mat4& projection);
	const Scene* scene_;
private:

	VkDescriptorSetLayout descriptorLayout_;

	VmaAllocator& allocator_;

	VkBuffer vertexBuffer_;
	VkBuffer indexBuffer_;
	VmaAllocation vertexBufferMem_;
	VmaAllocation indexBufferMem_;

	std::vector<RenderInfo> drawData_;
	std::vector<VkDeviceSize> vertexOffsets_;
	std::vector<VkDeviceSize> indexOffsets_;
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indicies_;
	VkDeviceSize globalVertexOffset_ = 0;
	VkDeviceSize globalIndexOffset_ = 0;

	PipelineBuilder::PipelineResources* debugPipeline_;
};
