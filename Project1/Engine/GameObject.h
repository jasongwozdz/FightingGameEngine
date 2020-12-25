#pragma once
#include "Renderer/Renderer.h"
#include "ResourceManager.h"
#include <memory>
#include <glm/glm.hpp>

class GameObject
{
public:
	GameObject() = default;
	GameObject(std::string modelPath, std::string texturePate, glm::vec3 position, VkDescriptorSetLayout layout);
	GameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, VkDescriptorSetLayout layout);
	virtual ~GameObject();
	void setPosition(glm::vec3 newPosition);
	void setScale(glm::vec3 newScale);
	Mesh* getMeshPtr();
	Mesh* p_mesh;

	virtual void placeHolder() {};
private:
	glm::vec3 position;
	glm::vec3 scale = {1, 1, 1};
};
