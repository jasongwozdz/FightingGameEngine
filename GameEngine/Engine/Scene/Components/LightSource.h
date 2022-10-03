#pragma once
#include <glm/glm.hpp>
#include "../Entity.h"
#include "../../EngineExport.h"
#include "../../Renderer/Asset/AssetTypes.h"

class ENGINE_API DirLight
{
public:
	DirLight() = default;

	DirLight(Entity* entity) :
		entity_(entity) {};

	Entity* entity_ = nullptr;
	DirLightUniformData uniformData_;
};

class ENGINE_API PointLight
{
public:
	PointLight() = default;

	PointLight(Entity* entity) :
		entity_(entity) {};

	Entity* entity_;
	PointLightUniformData uniformData_;
};
