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

//push constant info for debugPipeline_.  just contains the model to world matrix info that is passed into the vertex shader
struct PushConstantInfo
{
	glm::mat4 modelMatrix;
};

//push constant info for pickerPipline.  just contains the model to world matrix info thats passed into the vertex shader
struct PushConstantPickerVertexInfo
{
	glm::mat4 modelMatrix;
};

//fragment shader push constant info for pickerPipeline.  The current mouse pos is passed in and a uniqueId for the currently filledRect that is being drawn
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
	//Debug pipeline Methods
	void addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color);
	void drawRect(glm::vec3 pos, glm::vec3 color, float minX, float maxX, float minY, float maxY);
	void drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color);

	//PickerPipelipeline Methods
	void drawFilledRect(glm::vec3 pos, const glm::vec3& color,  glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, const int uniqueId);//This supports mouspicking.  The assigned uniqueId must be > 0 since the storageBuffer is by default populated with 0s
	bool getSelectedObject(int& id);//This will set id to whatever filled rect the mouse is hovering over in the current screen

	void renderFrame(const VkCommandBuffer& currentBuffer, const int currentImageIndex);//Called every frame in VkRenderer.cpp in draw()

	Scene* scene_;//used to grave the current projection matrix from the camera

	glm::vec2 mouseInfo_;//populated by client 
private:
	VmaAllocator& allocator_;

	//vulakn resources required for the mousePicking compatable drawFilledRect.  This pipelines topology is TRIANGLE
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
		VkBuffer storageBuffer_;
		VmaAllocation vertexBufferMem_;
		VmaAllocation indexBufferMem_;
		VmaAllocation storageBufferMem_;
		VkDescriptorSetLayout storageBufferDescriptorLayout_;
		VkDescriptorSet descriptorSet_;

		PipelineBuilder::PipelineResources* pipeline_;
	} pickerPipelineInfo;

	//Stores everything thats needed for the debugPipeline.  This pipeline's topology is set to LINE_LIST
	struct
	{
		std::vector<RenderInfo> drawData_;
		std::vector<VkDeviceSize> vertexOffsets_;
		std::vector<VkDeviceSize> indexOffsets_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indicies_;
		VkDeviceSize globalVertexOffset_ = 0;
		VkDeviceSize globalIndexOffset_ = 0;

		VkBuffer vertexBuffer_;
		VkBuffer indexBuffer_;
		VmaAllocation vertexBufferMem_;
		VmaAllocation indexBufferMem_;

		PipelineBuilder::PipelineResources* pipeline_;
	} debugPipelineInfo;
};
