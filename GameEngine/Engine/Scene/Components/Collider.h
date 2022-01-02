#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

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

	//std::function<void(BoxCollider*)> callbackFunc_;
	float width_, height_, length_;
	glm::vec3 position_;//pos in entitys local space
	uint32_t layer_;
};

struct Collider
{
	Collider(Entity* entity) : entity_(entity) {};
	std::vector<BoxCollider> colliders_;
	Entity* entity_;
};

