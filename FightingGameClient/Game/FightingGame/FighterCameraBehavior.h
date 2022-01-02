#pragma once
#include "BaseCamera.h"

#include <queue>

#include "Fighter.h"
#include "Scene/Components/Behavior.h"

class FighterCameraBehavior : public BehaviorImplementationBase 
{
public:
	FighterCameraBehavior(const Fighter* fighter1, const Fighter* fighter2);
	void update();

private:
	void calculateFov(glm::vec3& f1Pos, glm::vec3 f2Pos);
	glm::vec3 midPoint(const glm::vec3& p1, const glm::vec3& p2);
	//bool shouldAddNewCameraPos();

private:
	const Fighter* fighter1_;
	const Fighter* fighter2_;
	Camera& camera_;

	//std::queue<glm::vec3> posQueue;
	//glm::vec3 nextPos;
	//glm::vec3 basePos_;
	//const float cameraUpdateFrequency_ = 0.05f;
	//const float framesPerUpdate_ = 1;
	//const float clamp_ = 1.00f;
	//const float magFactor_ = 0.5f;
	//float lastUpdate_ = -1.0f;
	//float lowestX_ = 100;

};
