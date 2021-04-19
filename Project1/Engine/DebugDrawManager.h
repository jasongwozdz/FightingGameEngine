#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene/Scene.h"

class DebugDrawManager
{
public:
	DebugDrawManager();
	~DebugDrawManager();
	void init(Scene* scene);
	Entity* addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled = true, Entity* parent = nullptr);
	Entity* addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, Entity* parent = nullptr);
	Entity* drawRect(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, float minX, float maxX, float minY, float maxY, Entity* parent = nullptr);
	Entity* drawGrid(glm::vec3 color, bool depthEnabled);
	Entity* drawGrid(glm::vec3 position, int width, int height, glm::vec3 color, bool depthEnabled);
private:
	Scene* scene_;
	std::vector<Entity> entities_;
};
