#pragma once
#include <glm/glm.hpp>
#include <NewRenderer/Renderable.h>
#include <glm/gtx/quaternion.hpp>

class Transform 
{
public:
	float x_ = 1.0f;
	float y_ = 1.0f;
	float z_ = 1.0f;

	float scaleX_ = 1.0f;
	float scaleY_ = 1.0f;
	float scaleZ_ = 1.0f;

	float rotateX_ = 0.0f;
	float rotateY_ = 0.0f;
	float rotateZ_ = 0.0f;

	glm::mat4 rotationMatrix_ = glm::mat4(1.0f);

	glm::quat quaternion_ = { 1.0f, 1.0f, 1.0f, 0.0f };

	float drawDebugGui_ = false;

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

	void setScale(float scale);

	void drawDebugGui();

	void applyTransformToMesh(Renderable& mesh);
};

