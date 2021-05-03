#include "InputHandler.h"
#include <iostream>

void InputHandler::handleInputPressed(Events::KeyPressedEvent& e)
{
	if (side_ == Input::Side::leftSide)
	{
		switch (e.KeyCode)
		{
		case 87: //w
			currentMovementInput_.y += 1;
			currentAttackInput_ |= Input::InputMap::up;
			break;
		case 83: //s
			currentMovementInput_.y -= 1;
			currentAttackInput_ |= Input::InputMap::down;
			break;
		case 65: //a
			currentMovementInput_.x -= 1;
			currentAttackInput_ |= Input::InputMap::left;
			break;
		case 68: //d
			currentMovementInput_.x += 1;
			currentAttackInput_ |= Input::InputMap::right;
			break;
		case 49: //1
			currentAttackInput_ |= Input::InputMap::light;
			break;
		case 50: //2
			currentAttackInput_ |= Input::InputMap::medium;
			break;
		case 51: //3
			currentAttackInput_ |= Input::InputMap::strong;
			break;
		case 52: //4
			currentAttackInput_ |= Input::InputMap::ultra;
			break;
		}
		//std::cout << currentAttackInput_ << std::endl;
	}
	else
	{
		switch (e.KeyCode)
		{
		case 265: //up
			currentMovementInput_.y += 1;
			currentAttackInput_ |= Input::InputMap::up;
			break;
		case 264: //down
			currentMovementInput_.y -= 1;
			currentAttackInput_ |= Input::InputMap::down;
			break;
		case 263: //left
			currentMovementInput_.x -= 1;
			currentAttackInput_ |= Input::InputMap::left;
			break;
		case 262: //right
			currentMovementInput_.x += 1;
			currentAttackInput_ |= Input::InputMap::right;
			break;
		case 53: //1
			currentAttackInput_ |= Input::InputMap::light;
			break;
		case 54: //2
			currentAttackInput_ |= Input::InputMap::medium;
			break;
		case 55: //3
			currentAttackInput_ |= Input::InputMap::strong;
			break;
		case 56: //4
			currentAttackInput_ |= Input::InputMap::ultra;
			break;
		}
	}
}

void InputHandler::handleInputReleased(Events::KeyReleasedEvent& e)
{

	if (side_ == Input::Side::leftSide)
	{
		switch (e.KeyCode)
		{
		case 87: //w
			currentMovementInput_.y = 0;
			currentAttackInput_ &= ~Input::InputMap::up;
			break;
		case 83: //s
			currentMovementInput_.y = 0;
			currentAttackInput_ &= ~Input::InputMap::down;
			break;
		case 65: //a
			currentMovementInput_.x += 1;
			currentAttackInput_ &= ~Input::InputMap::left;
			break;
		case 68: //d
			currentMovementInput_.x -= 1;
			currentAttackInput_ &= ~Input::InputMap::right;
			break;
		case 49: //1
			currentAttackInput_ &= ~Input::InputMap::light;
			break;
		case 50: //2
			currentAttackInput_ &= ~Input::InputMap::medium;
			break;
		case 51: //3
			currentAttackInput_ &= ~Input::InputMap::strong;
			break;
		case 52: //4
			currentAttackInput_ &= ~Input::InputMap::ultra;
			break;
		}
		//std::cout << currentAttackInput_ << std::endl;
	}
	else
	{
		switch (e.KeyCode)
		{
		case 265: //up
			currentMovementInput_.y = 0;
			break;
		case 264: //down
			currentMovementInput_.y = 0;
			break;
		case 263: //left
			currentMovementInput_.x += 1;
			break;
		case 262: //right
			currentMovementInput_.x -= 1;
			break;
		case 53: //1
			currentAttackInput_ &= ~Input::InputMap::light;
			break;
		case 54: //2
			currentAttackInput_ &= ~Input::InputMap::medium;
			break;
		case 55: //3
			currentAttackInput_ &= ~Input::InputMap::strong;
			break;
		case 56: //4
			currentAttackInput_ &= ~Input::InputMap::ultra;
			break;
		}
	}
}

bool InputHandler::isInputCurrentlyPressed(Input::InputMap input)
{
	return (currentAttackInput_ & input);
}