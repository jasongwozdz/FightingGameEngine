#pragma once
#include "Renderer.h"
#include "ResourceManager.h"
#include <memory>

struct Position 
{
	float x;
	float y;
	float z;
};

class GameObject
{
public:
	GameObject(std::string modelPath, std::string texturePath, Position position);
	void setPosition(Position newPosition);
	Mesh* getMeshPtr();
private:
	Mesh* p_mesh;
	Position position;
};

