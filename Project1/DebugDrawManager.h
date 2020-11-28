#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "PrimitiveMesh.h"
#include "RenderPassComponent.h"

class GraphicsPipeline;

class DebugDrawManager
{
public:
	DebugDrawManager();
	~DebugDrawManager();
	void addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled = true);
	void addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled);
	void drawGrid(glm::vec3 color, bool depthEnabled);
private:
	std::vector<Mesh*> meshes;
	GraphicsPipeline* pipeline;
	GraphicsPipeline* linePipeline = nullptr;
	VkDescriptorSetLayout descriptorSetLayout; 
};
