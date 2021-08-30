#include "FighterCamera.h"
#include <ctime>
#include "NewRenderer/UIInterface.h"

#define CURR_TIME std::clock()/CLOCKS_PER_SEC

FighterCamera::FighterCamera(BaseCamera* camera, const Fighter* fighter1, const Fighter* fighter2) :
	fighter1_(fighter1), fighter2_(fighter2), camera_(camera)
{
	const float CAMERA_DISTANCE_FROM_FIGHTERS = 12.0f;
	const float CAMERA_HEIGHT = 2;

	posQueue.push(basePos_);
	glm::vec3 mid = midPoint(fighter1_->entity_->getComponent<Transform>().pos_, fighter2_->entity_->getComponent<Transform>().pos_);

	camera_->position = { mid.x - CAMERA_DISTANCE_FROM_FIGHTERS, mid.y, mid.z+CAMERA_HEIGHT };
	glm::vec3 direction = mid - camera_->position;
	camera_->viewDirection = direction;
};

glm::vec3 FighterCamera::midPoint(const glm::vec3& p1, const glm::vec3& p2)
{
	return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.y + p2.y) / 2 };
}

bool FighterCamera::shouldAddNewCameraPos()
{
	float dt = CURR_TIME - lastUpdate_;
	if (dt >= cameraUpdateFrequency_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void FighterCamera::onUpdate(float deltaTime)
{
	Entity& f1 = *fighter1_->entity_;
	Entity& f2 = *fighter2_->entity_;
	glm::vec3& f1pos = f1.getComponent<Transform>().pos_;
	glm::vec3& f2pos = f2.getComponent<Transform>().pos_;
	glm::vec3 mid = midPoint(f1pos, f2pos);
	camera_->position.y = mid.y;
}