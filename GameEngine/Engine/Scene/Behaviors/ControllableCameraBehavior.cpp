#include "ControllableCameraBehavior.h"
#include "../../Input.h"
#include "../Components/Camera.h"

ControllableCameraBehavior::ControllableCameraBehavior(Entity* entity) :
	oldMousePos_({ 0.0f, 0.0f }),
	BehaviorImplementationBase(entity),
	input_(Input::getSingletonPtr())
{
	entity_->getComponent<Camera>().fovAngleInDegrees_ = 60.0f;
}

void ControllableCameraBehavior::update()
{
	handleKeyInput();
	handleMouseInput();
}

void ControllableCameraBehavior::handleKeyInput()
{
	if (input_->getButton("LeftClick") > 0.0f)
	{
		Transform& transform = entity_->getComponent<Transform>();
		transform.position_ += transform.forward() * input_->getAxis("Vertical") * cameraSpeed_;
		transform.position_ += transform.left() * input_->getAxis("Horizontal") * cameraSpeed_;
	}
}

void ControllableCameraBehavior::handleMouseInput()
{
	glm::vec2 currentMousePos = input_->mousePos_;
	Transform& transform = entity_->getComponent<Transform>();
	if (input_->getButton("RightClick") > 0.0f)
	{
		glm::vec2 diff = currentMousePos - oldMousePos_;
		transform.rotateAround(diff.y * rotationSpeed_, Transform::worldLeft);
		transform.rotateAround(diff.x * rotationSpeed_, Transform::worldUp);
	}
	oldMousePos_ = currentMousePos;
}
