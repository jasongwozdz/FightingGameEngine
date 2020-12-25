#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
class Transform
{
public:
	float x_;
	float y_;
	float z_;

	Transform(float x, float y, float z);

	Transform& operator +(const Transform& other)
	{
		x_ += other.x_;
		y_ += other.y_;
		z_ += other.z_;
	}

	operator const glm::vec3() 
	{
		glm::vec3 v(x_, y_, z_);
		return v;
	}

	void applyTransformToMesh(Mesh& mesh);
};

