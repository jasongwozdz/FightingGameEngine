#pragma once
#include "glm/glm.hpp"
#include "Scene/Entity.h"

struct Arena
{
	glm::vec3 size;
	glm::vec3 pos;
	Entity* entity;
	std::vector<Entity*> walls;
};

