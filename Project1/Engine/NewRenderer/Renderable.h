#pragma once
#include <vector>
#include <string>
#include "../Vertex.h"

#define MAX_BONES 64

struct Ubo 
{
	alignas(16)glm::mat4 model;
	alignas(16)glm::mat4 view;
	alignas(16)glm::mat4 proj;
	alignas(16)glm::mat4 bones[MAX_BONES];
};

class Renderable
{
public:

	Renderable(std::vector<Vertex> vertex, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName);

	Renderable(Renderable&& other);

	Renderable(const Renderable& other);

	Renderable& operator=(Renderable const& other)
	{
		depthEnabled_ = false;
		draw_ = false;
		uploaded_ = false;
		delete_ = false;
		isLine_ = false;
		ubo_ = new Ubo;
		memcpy(ubo_, &other.ubo_, sizeof(other.ubo_));
		return *this;
	}

	~Renderable();
	
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	std::string entityName_;
	
	Ubo& ubo();
	bool depthEnabled_;
	bool draw_;
	bool uploaded_;
	bool delete_;
	bool isLine_ = false;
private:
	Ubo* ubo_;
};
