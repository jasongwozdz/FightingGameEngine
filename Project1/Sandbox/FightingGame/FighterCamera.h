#pragma once
#include "BaseCamera.h"
#include "Fighter.h"
#include <queue>

class FighterCamera
{
public:
	FighterCamera(BaseCamera* camera, const Fighter* fighter1, const Fighter* fighter2);

	//editable
	const float cameraUpdateFrequency_ = 0.05f;
	const float framesPerUpdate_ = 1;

	glm::vec3 basePos_;
	const float clamp_ = 1.00f;
	const float magFactor_ = 0.5f;

	float lastUpdate_ = -1.0f;

	float lowestX_ = 100;

	BaseCamera* camera_;
	const Fighter* fighter1_;
	const Fighter* fighter2_;

	std::queue<glm::vec3> posQueue;

	glm::vec3 nextPos;

	glm::vec3 midPoint(const glm::vec3& p1, const glm::vec3& p2);

	float calculateX(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& mid);

	bool shouldAddNewCameraPos();

	void onUpdate(float deltaTime);
};
