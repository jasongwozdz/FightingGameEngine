#include <math.h>
#include <vector>
#include "DebugDrawManager.h"
#include "ResourceManager.h"

std::string sphereModelLoc = "models/geosphere.obj";

DebugDrawManager::DebugDrawManager()
{
}

void DebugDrawManager::init(Scene* scene)
{
	scene_ = scene;
}

DebugDrawManager::~DebugDrawManager()
{
}

Entity& DebugDrawManager::addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled)
{

	ModelReturnVals	returnVals = ResourceManager::getSingleton().loadObjFile(sphereModelLoc);
	
	Entity& point = scene_->addEntity("point");
	Renderable& r = point.addComponent<Renderable>(returnVals.vertices, returnVals.indices, true, "point");
	Transform& t = point.addComponent<Transform>(pos);
	return point;
}

void DebugDrawManager::addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled)
{
	std::vector<Vertex> vertices = {
		{fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
		{toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
	};
	
	std::vector<uint32_t> indices = { 0, 1 };
}

Entity& DebugDrawManager::drawGrid(glm::vec3 color, bool depthEnabled)
{
	glm::vec3 x1 = { 50, -50, 0 };
	glm::vec3 x2 = { -50, -50, 0 };
	glm::vec3 y1 = { -50, 50, 0 };
	glm::vec3 y2 = { -50, -50, 0 };

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < 101; ++i)
	{
		vertices.push_back({ x1,color });
		vertices.push_back({ x2,color });
		indices.push_back(i*4);
		indices.push_back(i*4 + 1);

		vertices.push_back({ y1,color });
		vertices.push_back({ y2,color });
		indices.push_back(i*4 + 2);
		indices.push_back(i*4 + 3);
		
		x1.y++;
		x2.y++;
		y1.x++;
		y2.x++;
	}

	Entity& grid = scene_->addEntity("debug_grid");
	Renderable& r = grid.addComponent<Renderable>(vertices, indices, true, "debug_grid");
	r.isLine_ = true;
	Transform& t = grid.addComponent<Transform>(0.0f, 0.0f, 0.0f);
	entities_.push_back(grid);
	return grid;
}
