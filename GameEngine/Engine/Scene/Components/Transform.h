#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../Renderer/Renderable.h"
#include "../Entity.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Transform 
{
public:
	Transform(float x, float y, float z);

	Transform(glm::vec3 pos);

	void setScale(float scale);

	static glm::vec3 worldUp;
	static glm::vec3 worldLeft;
	static glm::vec3 worldForward;

	glm::vec3 left();
	glm::vec3 up();
	glm::vec3 forward();

	void drawDebugGui();

	glm::mat4 calculateTransform();
	void applyTransformToMesh(Renderable& mesh);

	//direction to point forward vector at
	void lookAt(glm::vec3 direction);
	//rotation around each local axis
	void rotateAround(glm::vec3 rotationInRadians);
	void rotateAround(float angleToRotateInDegrees, glm::vec3 axisToRotateAround);

public:
	glm::mat4 finalTransform_ = glm::mat4(1.0f);

	glm::vec3 position_ = {0.0f, 0.0f, 0.0f};

	glm::vec3 scale_ = {1.0f, 1.0f, 1.0f};

	glm::quat rotation_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	bool drawDebugGui_ = false;
	bool drawDebug_ = false;

private:
	void drawDebug();

private:
	glm::vec3 oldPos_;
	glm::vec3 oldScale_;
	glm::quat oldRot_;
	bool calculateTransform_ = true;
};
