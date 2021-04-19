#include "InputHandler.h"

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
		}
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
		}
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
		}
	}
}

glm::vec2& InputHandler::getInput()
{
	return currentInput_;
}
