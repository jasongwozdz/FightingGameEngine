#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <math.h>

#include "../../Renderer/Renderable.h"

class Entity;

struct BoxCollider
{
	BoxCollider(uint32_t width, uint32_t height, uint32_t length, glm::vec3 pos, uint32_t layer) :
		width_(width),
		height_(height),
		length_(length),
		position_(pos),
		layer_(layer)
	{};

	BoxCollider(Renderable& mesh)//generate optimal bouding box for a mesh
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		for (Vertex vertex : mesh.vertices_)
		{
			glm::vec3 pos = vertex.pos;
			if(pos.x < min.x) min.x = pos.x;
			if(pos.x > max.x) max.x = pos.x;
			if(pos.y < min.y) min.y = pos.y;
			if(pos.y > max.y) max.y = pos.y;
			if(pos.z < min.z) min.z = pos.z;
			if(pos.z > max.z) max.z = pos.z;
		}
		width_ = max.x - min.x;
		height_ = max.y - min.y;
		length_ = max.z - min.z;
		position_ = glm::vec3(0.0f);
		layer_ = 0;
	}
	//std::function<void(BoxCollider*)> callbackFunc_;
	float width_, height_, length_;
	glm::vec3 position_;//pos in entitys local space
	uint32_t layer_;
};

struct Collider
{
	Collider(Entity* entity) : entity_(entity) {};

	Collider(Entity* entity, float width, float height, float length, glm::vec3 position, uint32_t layer) : 
		entity_(entity) 
	{
		BoxCollider collider(width, height, length, position, layer);
		colliders_.push_back(collider);
	};

	std::vector<BoxCollider> colliders_;
	Entity* entity_;
};

