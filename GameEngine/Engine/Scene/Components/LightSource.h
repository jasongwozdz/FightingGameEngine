#pragma once
#include <glm/glm.hpp>
#include "../Entity.h"
#include "../../EngineExport.h"

class ENGINE_API LightSource
{
public:
	LightSource(Entity* entity);

	struct DirLightUniformData {
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 ambient;
		alignas(16) glm::vec3 diffuse;
		alignas(16) glm::vec3 specular;
	};
	Entity* entity_;
	DirLightUniformData uniformData_;
};
