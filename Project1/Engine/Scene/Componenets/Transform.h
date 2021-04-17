#pragma once
#include <glm/glm.hpp>
#include <NewRenderer/Renderable.h>
#include <glm/gtx/quaternion.hpp>

class Transform 
{
public:
	Transform(float x, float y, float z);

	Transform(glm::vec3 pos);

	float x_ = 1.0f;
	float y_ = 1.0f;
	float z_ = 1.0f;

	float scaleX_ = 1.0f;
	float scaleY_ = 1.0f;
	float scaleZ_ = 1.0f;

	glm::quat quaternion_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	float drawDebugGui_ = false;

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

	glm::mat4 calculateTransform();

	void applyTransformToMesh(Renderable& mesh);

	glm::vec3 getPosition();

	void setPosition(glm::vec3 pos);

	void flipScale();

	void applyAxisAngleRotation(float rotInDegrees, glm::vec3 axis);
};

