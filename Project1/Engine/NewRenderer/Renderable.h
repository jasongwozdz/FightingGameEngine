#pragma once
#include <vector>
#include <string>
#include "../Vertex.h"

#define MAX_BONES 64

class Renderable
{
public:

	Renderable(std::vector<Vertex> vertex, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName);

	~Renderable() = default;

	struct
	{
		alignas(16)glm::mat4 model;
		alignas(16)glm::mat4 view;
		alignas(16)glm::mat4 proj;
	}ubo_;
	
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	std::string entityName_;
	bool depthEnabled_;
	bool draw_;
	bool uploaded_;
	bool delete_;
};
