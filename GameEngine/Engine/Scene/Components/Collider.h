#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <math.h>

#include "../../Renderer/Renderable.h"
#include "Transform.h"
#include "Collider.h"

class Entity;

typedef void(BoxColliderCallback)(Entity* otherEnt, struct BoxCollider* thisCollider, class BoxCollider* otherCollider);
#define BIND_COLLIDER_CALLBACK(function) std::bind(&function, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

struct BoxCollider
{
	BoxCollider(glm::vec3 size, glm::vec3 pos, uint32_t layer, Entity* entity) :
		size_(size),
		position_(pos),
		layer_(layer),
		callback_(nullptr)
	{};

	BoxCollider() : 
		size_(glm::vec3(0)),
		position_(glm::vec3(0)),
		layer_(0),
		callback_(nullptr),
		entity_(nullptr)
	{};

	BoxCollider(Renderable& mesh, Entity* entity)//generate optimal bouding box for a mesh
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		for (Vertex vertex : mesh.vertices_)
		{
			Transform& transform = entity->getComponent<Transform>();
			glm::vec3 pos = transform.calculateTransformNoScale() * glm::vec4(vertex.pos, 1.0f);

			if(pos.x < min.x) min.x = pos.x;
			if(pos.x > max.x) max.x = pos.x;
			if(pos.y < min.y) min.y = pos.y;
			if(pos.y > max.y) max.y = pos.y;
			if(pos.z < min.z) min.z = pos.z;
			if(pos.z > max.z) max.z = pos.z;
		}
		size_.x = max.x - min.x;
		size_.y = max.y - min.y;
		size_.z = max.z - min.z;
		position_ = (min + max) * .5f;
		layer_ = 0;
	}

	glm::vec3 size_;
	glm::vec3 position_;//pos in entitys local space
	uint32_t layer_;
	std::function<BoxColliderCallback> callback_;
	Entity* entity_;
};

class Collider
{
public:
	Collider(Entity* entity) : entity_(entity), collidersOutOfDate_(false) {};

	Collider(Entity* entity, glm::vec3 size, glm::vec3 position, uint32_t layer) :
		entity_(entity),
		collidersOutOfDate_(false)
	{
		BoxCollider collider(size, position, layer, entity_);
		colliders_.push_back(collider);
	};

	void setColliders(std::vector<BoxCollider>& newColliders)
	{
		collidersOutOfDate_ = true;
		colliders_ = newColliders;
	}

	std::vector<BoxCollider> colliders_;
	Entity* entity_;
	bool collidersOutOfDate_;
};

