#include "Fighter.h"
#include "ResourceManager.h"

Fighter::Fighter(Entity& entity, Transform& transform, Animator& animator, InputHandler& inputHandler) :
	entity_(entity), transform_(transform), animator_(animator), inputHandler_(inputHandler)
{}

void Fighter::updateTransform()
{
	glm::vec3 trans = { 0, currentMovement_.x * speed_, 0};

	Transform& transform = entity_.getComponent<Transform>();

	glm::vec3 currentPos = transform.getPosition();

	currentPos += trans;

	std::cout << "Z: " << currentPos.z << std::endl;

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
		animator_.setAnimation(-1);
		break;
	case FighterState::walking:
		animator_.setAnimation(0);
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
