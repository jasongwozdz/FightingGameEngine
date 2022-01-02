#include "FighterCameraBehavior.h"

#include <ctime>

#include "Renderer/UIInterface.h"
#include "Scene/Components/Camera.h"

#define CURR_TIME std::clock()/CLOCKS_PER_SEC

FighterCameraBehavior::FighterCameraBehavior(const Fighter* fighter1, const Fighter* fighter2) :
	fighter1_(fighter1), fighter2_(fighter2), camera_(Scene::getSingleton().getCurrentCamera()), BehaviorImplementationBase(Scene::getSingleton().getCurrentCamera().entity_)
{
	const float CAMERA_DISTANCE_FROM_FIGHTERS = 12.0f;
	const float CAMERA_HEIGHT = 2;

	glm::vec3 mid = midPoint(fighter1_->entity_->getComponent<Transform>().position_, fighter2_->entity_->getComponent<Transform>().position_);

	Transform& cameraTransform = camera_.entity_->getComponent<Transform>();
	cameraTransform.lookAt(mid - cameraTransform.position_);
	 //cameraTransform.position_ = { mid.x - CAMERA_DISTANCE_FROM_FIGHTERS, mid.y, mid.z+CAMERA_HEIGHT };

};

glm::vec3 FighterCameraBehavior::midPoint(const glm::vec3& p1, const glm::vec3& p2)
{
	return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2 };
}

//bool FighterCameraBehavior::shouldAddNewCameraPos()
//{
//	float dt = CURR_TIME - lastUpdate_;
//	if (dt >= cameraUpdateFrequency_)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

void FighterCameraBehavior::calculateFov(glm::vec3& f1Pos, glm::vec3 f2Pos)
{
	glm::vec3 viewEdge;
	Transform& transform = camera_.entity_->getComponent<Transform>();
	if (glm::distance(f1Pos, transform.position_) > glm::distance(f2Pos, transform.position_))
	{
		viewEdge = f1Pos - transform.position_;
	}
	else
	{
		viewEdge = f2Pos - transform.position_;
	}
	float hyp = viewEdge.length();
	float adj = glm::dot(viewEdge, transform.forward());
	float angleRad = glm::acos(adj / hyp);
	camera_.fovAngleInDegrees_ = glm::degrees(angleRad);
}

void FighterCameraBehavior::update()
{
	Transform& fighter1Transform = fighter1_->entity_->getComponent<Transform>();
	Transform& fighter2Transform = fighter2_->entity_->getComponent<Transform>();
	glm::vec3 mid = midPoint(fighter1Transform.position_, fighter2Transform.position_);
	Transform& cameraTransform = camera_.entity_->getComponent<Transform>();
	cameraTransform.lookAt(mid - cameraTransform.position_);

	//Entity& f1 = *fighter1_->entity_;
	//Entity& f2 = *fighter2_->entity_;
	//glm::vec3& f1pos = f1.getComponent<Transform>().position_;
	//glm::vec3& f2pos = f2.getComponent<Transform>().position_;
	//glm::vec3 mid = midPoint(f1pos, f2pos);
	//camera_->position.y = mid.y;
	//calculateFov(f1pos, f2pos);
}
