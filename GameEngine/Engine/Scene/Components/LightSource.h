#pragma once
#include <glm/glm.hpp>
#include "../Entity.h"
#include "../../EngineExport.h"

class ENGINE_API DirLight
{
public:
	DirLight() = default;

	DirLight(Entity* entity) :
		entity_(entity) {};

	struct DirLightUniformData {
		alignas(16) glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 ambient = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 diffuse = { 0.0f, 0.0f, 0.0f } ;
		alignas(16) glm::vec3 specular = { 0.0f, 0.0f, 0.0f } ;
	};
	Entity* entity_ = nullptr;
	DirLightUniformData uniformData_;
};

class ENGINE_API PointLight
{
public:
	PointLight() = default;

	PointLight(Entity* entity) :
		entity_(entity) {};

	struct PointLightUniformData 
	{
		alignas(4)  float constant = 1.0f;
		alignas(4)  float linear = 0.09f;
		alignas(4)  float quadratic = 0.03f;
		alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 ambient = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 diffuse = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 specular = { 0.0f, 0.0f, 0.0f };
	};

	Entity* entity_;
	PointLightUniformData uniformData_;
};
