#pragma once
#include <glm/glm.hpp>
#include "../Entity.h"

class LightSource
{
public:
	struct DirLightUniformData {
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 ambient;
		alignas(16) glm::vec3 diffuse;
		alignas(16) glm::vec3 specular;
	};

	LightSource(Entity* entity);
	void uploadLightSourceData();

private:
	glm::vec3 color_;
	glm::vec3 diffuse_;
	glm::vec3 ambient_;
	glm::vec3 specular_;
	DirLightUniformData uniformData_;

	Entity* entity_;
};
