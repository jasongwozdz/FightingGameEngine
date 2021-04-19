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

/*
 p1-------------p2
 |				 |
 |				 |
 |				 |
 |				 |
 |				 |
 p3-------------p4
*/
Entity* DebugDrawManager::drawRect(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, float minX, float maxX, float minY, float maxY, Entity* parent)
{
	glm::vec3 p1 = { 0.0f, minX, maxY };
	glm::vec3 p2 = { 0.0f, maxX, maxY };
	glm::vec3 p3 = { 0.0f, minX, minY };
	glm::vec3 p4 = { 0.0f, maxX, minY };
	std::vector<Vertex> vertices;
	vertices.push_back({ p1, color});
	vertices.push_back({ p2, color});
	vertices.push_back({ p3, color});
	vertices.push_back({ p4, color});

	std::vector<uint32_t> indices;
	indices.push_back(0);
	indices.push_back(1);

	indices.push_back(1);
	indices.push_back(3);

	indices.push_back(3);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(0);

	Entity* rect = scene_->addEntity("rect");
	Renderable& r = rect->addComponent<Renderable>(vertices, indices, true, "rect");
	r.isLine_ = true;
	Transform& t = rect->addComponent<Transform>(pos);
	t.parent_ = parent;
	return rect;
}

Entity* DebugDrawManager::addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled, Entity* parent)
{
	ModelReturnVals	returnVals = ResourceManager::getSingleton().loadObjFile(sphereModelLoc);
	Entity* point = scene_->addEntity("point");
	Renderable& r = point->addComponent<Renderable>(returnVals.vertices, returnVals.indices, true, "debug_line");
	Transform& t = point->addComponent<Transform>(pos);
	t.setScale(5.f);
	t.parent_ = parent;
	return point;
}

Entity* DebugDrawManager::addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled, Entity* parent)
{
	std::vector<Vertex> vertices = {
		{fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
		{toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
	};
	
	std::vector<uint32_t> indices = { 0, 1 };

	Entity* line = scene_->addEntity("debug_line");
	Renderable& r = line->addComponent<Renderable>(vertices, indices, true, "debug_line");
	r.isLine_ = true;
	Transform& t = line->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	t.parent_ = parent;
	return line;
}

Entity* DebugDrawManager::drawGrid(glm::vec3 position, int width, int height, glm::vec3 color, bool depthEnabled)
{
	glm::vec3 p1 = { -width / 2, -height / 2, 0 };
	glm::vec3 p2 = { width / 2, -height / 2, 0 };

	glm::vec3 p3 = {-width / 2, -height / 2, 0};
	glm::vec3 p4 = {-width / 2, height / 2, 0};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < height; i++)
	{
		vertices.push_back({p1, color});
		indices.push_back(i * 2);
		vertices.push_back({p2, color});
		indices.push_back(i * 2 + 1);

		p1.y++;
		p2.y++;
	}

	for (int i = 0; i < width; i++)
	{
		vertices.push_back({p3, color});
		indices.push_back((height*2) + (i * 2));
		vertices.push_back({p4, color});
		indices.push_back((height*2) + (i * 2 + 1));

		p3.x++;
		p4.x++;
	}

	Entity* grid = scene_->addEntity("debug_grid");
	Renderable& r = grid->addComponent<Renderable>(vertices, indices, true, "debug_grid");
	r.isLine_ = true;
	Transform& t = grid->addComponent<Transform>(position);
	return grid;
}

Entity* DebugDrawManager::drawGrid(glm::vec3 color, bool depthEnabled)
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

	Entity* grid = scene_->addEntity("debug_grid");
	Renderable& r = grid->addComponent<Renderable>(vertices, indices, true, "debug_grid");
	r.isLine_ = true;
	Transform& t = grid->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	return grid;
}
