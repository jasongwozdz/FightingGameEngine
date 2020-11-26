#pragma once
#include "Renderer.h"
#include "ResourceManager.h"
#include <memory>
#include <glm/glm.hpp>

class GameObject
{
public:
	GameObject(std::string modelPath, std::string texturePate, glm::vec3 position);
	GameObject(Mesh* mesh, glm::vec3 pos);
	void setPosition(glm::vec3 newPosition);
	Mesh* getMeshPtr();
private:
	Mesh* p_mesh;
	glm::vec3 position;
};
