#include "InputHandler.h"
#include <iostream>

void InputHandler::handleInputPressed(Events::KeyPressedEvent& e)
{
	if (side_ == Input::Side::left)
	{
		switch (e.KeyCode)
		{
		case 87: //w
			currentInput_.y += 1;
			break;
		case 83: //s
			currentInput_.y -= 1;
			break;
		case 65: //a
			currentInput_.x -= 1;
			break;
		case 68: //d
			currentInput_.x += 1;
			break;
		case 49: //1
			currentAttackInput_ |= Input::AttackMap::light;
			break;
		case 50: //2
			currentAttackInput_ |= Input::AttackMap::medium;
			break;
		case 51: //3
			currentAttackInput_ |= Input::AttackMap::strong;
			break;
		case 52: //4
			currentAttackInput_ |= Input::AttackMap::ultra;
			break;
		}
		//std::cout << currentAttackInput_ << std::endl;
	}
	else
	{
		switch (e.KeyCode)
		{
		case 265: //up
			currentInput_.y += 1;
			break;
		case 264: //down
			currentInput_.y -= 1;
			break;
		case 263: //left
			currentInput_.x -= 1;
			break;
		case 262: //right
			currentInput_.x += 1;
			break;
		case 53: //1
			currentAttackInput_ |= Input::AttackMap::light;
			break;
		case 54: //2
			currentAttackInput_ |= Input::AttackMap::medium;
			break;
		case 55: //3
			currentAttackInput_ |= Input::AttackMap::strong;
			break;
		case 56: //4
			currentAttackInput_ |= Input::AttackMap::ultra;
			break;
		}
	}
}

void InputHandler::handleInputReleased(Events::KeyReleasedEvent& e)
{

	if (side_ == Input::Side::left)
	{
		switch (e.KeyCode)
		{
		case 87: //w
		case 83: //s
			currentInput_.y = 0;
			break;
		case 65: //a
			currentInput_.x += 1;
			break;
		case 68: //d
			currentInput_.x -= 1;
			break;
		case 49: //1
			currentAttackInput_ &= Input::AttackMap::notLight;
			break;
		case 50: //2
			currentAttackInput_ &= Input::AttackMap::notMedium;
			break;
		case 51: //3
			currentAttackInput_ &= Input::AttackMap::notStrong;
			break;
		case 52: //4
			currentAttackInput_ &= Input::AttackMap::notUltra;
			break;
		}
		//std::cout << currentAttackInput_ << std::endl;
	}
	else
	{
		switch (e.KeyCode)
		{
		case 265: //up
		case 264: //down
			currentInput_.y = 0;
			break;
		case 263: //left
			currentInput_.x += 1;
			break;
		case 262: //right
			currentInput_.x -= 1;
			break;
		case 53: //1
			currentAttackInput_ &= Input::AttackMap::notLight;
			break;
		case 54: //2
			currentAttackInput_ &= Input::AttackMap::notMedium;
			break;
		case 55: //3
			currentAttackInput_ &= Input::AttackMap::notStrong;
			break;
		case 56: //4
			currentAttackInput_ &= Input::AttackMap::notUltra;
			break;
		}
	}
}
