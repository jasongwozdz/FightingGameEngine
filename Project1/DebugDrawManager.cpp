#include <vector>
#include "DebugDrawManager.h"
#include "GameObjectManager.h"

DebugDrawManager::DebugDrawManager() {};
DebugDrawManager::~DebugDrawManager() {};

void DebugDrawManager::addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled)
{
	GameObjectManager& manager = GameObjectManager::getSingleton();
	Renderer& renderer = Renderer::getSingleton();
	float len = glm::length(toPos - fromPos);
	std::vector<Vertex> verticies = {
		{fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
		{fromPos+lineWidth, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
		{toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
		{toPos-lineWidth, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } }
	};

	/*Vertex v1 = { fromPos, color,{0.0, 0.0}, {0.0, 0.0, 0.0} };
	Vertex v2 = { toPos, color, {0.0, 0.0}, {0.0, 0.0, 0.0} };
	Vertex v3 = { fromPos - toPos, color, {0.0, 0.0}, {0.0, 0.0, 0.0} };
	std::vector<Vertex> verticies = { v1, v2, v3 };*/
	std::vector<uint32_t>  indicies = {0, 1, 2, 2, 3, 0};

	Mesh* mesh = new PrimitiveMesh(verticies, indicies, renderer.getCommandBuffers(), renderer.getLogicalDevice(), renderer.getSwapChainImages(), renderer.getSwapChainExtent(), renderer.getPhysicalDevice(), renderer.getCommandPool(), renderer.getGraphicsQueue());
	meshes.push_back(mesh);
	pipeline = new GraphicsPipeline(renderer.getLogicalDevice(), renderer.getRenderPass(),mesh->m_descriptorSetLayout, renderer.getSwapChainExtent(), "shaders/vert.spv", "shaders/frag.spv", renderer.getDepthBufferComp(), false);
	GameObject* object = new GameObject(mesh, fromPos);
	manager.addGameObject(object, pipeline);
}
