#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../NewRenderer/Renderable.h"
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
	
	glm::vec3 pos_ = {0.0f, 0.0f, 0.0f};
	glm::vec3 oldPos_ = { 0.0f, 0.0f, 0.0f};

	glm::vec3 scale_ = {1.0f, 1.0f, 1.0f};
	glm::vec3 oldScale_ = {1.0f, 1.0f, 1.0f};

	glm::quat rot_ = { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::quat oldRot_ = { 0.0f, 0.0f, 0.0f, 0.0f };

	glm::vec3 forwardDirection_ = { 0.0f, 0.0f, 0.0f };

	Entity* parent_ = nullptr;

	glm::mat4 finalTransform_ = glm::mat4(1.0f);

	float drawDebugGui_ = false;

	bool calculateTransform_ = true;

	void setScale(float scale);

	void drawDebugGui();

	glm::mat4 calculateTransform();

	void applyTransformToMesh(Renderable& mesh);
};

