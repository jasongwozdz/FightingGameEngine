#pragma once
#include "glm/glm.hpp"
#include "Scene/Entity.h"

struct Arena
{
	float width, depth, length;
	glm::vec3 pos;
	Entity* entity;
};

