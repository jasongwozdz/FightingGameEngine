#pragma once
#include "Renderer.h"
#include "ResourceManager.h"
#include <memory>
#include <glm/glm.hpp>

class GameObject
{
public:
	GameObject(std::string modelPath, std::string texturePate, glm::vec3 position, VkDescriptorSetLayout layout);
	GameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, VkDescriptorSetLayout layout);
	~GameObject();
	void setPosition(glm::vec3 newPosition);
	Mesh* getMeshPtr();
private:
	Mesh* p_mesh;
	glm::vec3 position;
};
