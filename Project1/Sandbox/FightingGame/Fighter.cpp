#include "Fighter.h"
#include "ResourceManager.h"

Fighter::Fighter(Entity& entity, InputHandler& inputHandler) :
	entity_(entity), inputHandler_(inputHandler)
{}

void Fighter::setPosition(glm::vec3 pos)
{
	entity_.getComponent<Transform>().setPosition(pos);
}

void Fighter::flipSide()
{
	entity_.getComponent<Transform>().scaleX_ *= -1;
	entity_.getComponent<Transform>().scaleY_ *= -1;
	entity_.getComponent<Transform>().scaleZ_ *= -1;
	glm::quat rot = entity_.getComponent<Transform>().quaternion_;
	glm::quat flipRot(0.0f, 0.0f, 1.0f, 0.0f);
	entity_.getComponent<Transform>().quaternion_ = flipRot * rot;
}

void Fighter::updateTransform()
{
	glm::vec3 trans = { 0, currentMovement_.x * speed_, 0};

	Transform& transform = entity_.getComponent<Transform>();

	glm::vec3 currentPos = transform.getPosition();

	currentPos += trans;

	transform.setPosition(currentPos);
}

void Fighter::handleState()
{
	switch (state_)
	{
	case FighterState::idle:
		handleMove();
		break;
	case FighterState::walking:
		handleMove();
		updateTransform();
		break;
	case FighterState::attacking:
		break;
	}
}

void Fighter::handleMove()
{
	if (controllable_)
	{
		if (currentMovement_.y == 0 && currentMovement_.x != 0)
		{
			setOrKeepState(FighterState::walking);
		}
		else
		{
			setOrKeepState(FighterState::idle);
		}
	}
}

void Fighter::setOrKeepState(FighterState state)
{
	if (state_ == state) return;
	else
	{
		state_ = state;
		enterState(state_);
	}
}

void Fighter::enterState(FighterState state)
{
	switch (state)
	{
	case FighterState::idle:
		entity_.getComponent<Animator>().setAnimation(-1);
		break;
	case FighterState::walking:
		entity_.getComponent<Animator>().setAnimation(0);
		break;
	}
}

void Fighter::processInput()
{
	currentMovement_ = inputHandler_.getInput();
}

void Fighter::onUpdate(float delta)
{
	if (controllable_)
	{
		processInput();
	}

	handleState();

}
