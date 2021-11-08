#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler(Input::Side side) : side_(side){}

void InputHandler::handleInputPressed(Events::KeyPressedEvent& e)
{

	uint8_t inputToQueue = 0;
	if (side_ == Input::Side::leftSide)
	{
		switch (e.KeyCode)
		{
		case 87: //w
			currentMovementInput_.y += 1;
			currentAttackInput_ |= inputToQueue = Input::Action::up;
			break;
		case 83: //s
			currentMovementInput_.y -= 1;
			currentAttackInput_ |= inputToQueue = Input::Action::down;
			break;
		case 65: //a
			currentMovementInput_.x -= 1;
			currentAttackInput_ |= inputToQueue = Input::Action::left;
			break;
		case 68: //d
			currentMovementInput_.x += 1;
			currentAttackInput_ |= inputToQueue = Input::Action::right;
			break;
		case 49: //1
			currentAttackInput_ |= inputToQueue = Input::Action::light;
			break;
		case 50: //2
			currentAttackInput_ |= inputToQueue = Input::Action::medium;
			break;
		case 51: //3
			currentAttackInput_ |= inputToQueue = Input::Action::strong;
			break;
		case 52: //4
			currentAttackInput_ |= inputToQueue = Input::Action::ultra;
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
			currentAttackInput_ |= inputToQueue = Input::Action::up;
			break;
		case 264: //down
			currentMovementInput_.y -= 1;
			currentAttackInput_ |= inputToQueue = Input::Action::down;
			break;
		case 263: //left
			currentMovementInput_.x -= 1;
			currentAttackInput_ |= inputToQueue = Input::Action::left;
			break;
		case 262: //right
			currentMovementInput_.x += 1;
			currentAttackInput_ |= inputToQueue = Input::Action::right;
			break;
		case 53: //5
			currentAttackInput_ |= inputToQueue = Input::Action::light;
			break;
		case 54: //6
			currentAttackInput_ |= inputToQueue = Input::Action::medium;
			break;
		case 55: //7
			currentAttackInput_ |= inputToQueue = Input::Action::strong;
			break;
		case 56: //56
			currentAttackInput_ |= inputToQueue = Input::Action::ultra;
			break;
		}
	}

	if (inputToQueue)
	{
		//add current input to queue
		Input::InputTime input = { Input::Action(inputToQueue), INPUT_TIME_IN_QUEUE };
		inputQueue_.push_back(input);
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
			currentAttackInput_ &= ~Input::Action::up;
			break;
		case 83: //s
			currentMovementInput_.y = 0;
			currentAttackInput_ &= ~Input::Action::down;
			break;
		case 65: //a
			currentMovementInput_.x += 1;
			currentAttackInput_ &= ~Input::Action::left;
			break;
		case 68: //d
			currentMovementInput_.x -= 1;
			currentAttackInput_ &= ~Input::Action::right;
			break;
		case 49: //1
			currentAttackInput_ &= ~Input::Action::light;
			break;
		case 50: //2
			currentAttackInput_ &= ~Input::Action::medium;
			break;
		case 51: //3
			currentAttackInput_ &= ~Input::Action::strong;
			break;
		case 52: //4
			currentAttackInput_ &= ~Input::Action::ultra;
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
			currentAttackInput_ &= ~Input::Action::light;
			break;
		case 54: //2
			currentAttackInput_ &= ~Input::Action::medium;
			break;
		case 55: //3
			currentAttackInput_ &= ~Input::Action::strong;
			break;
		case 56: //4
			currentAttackInput_ &= ~Input::Action::ultra;
			break;
		}
	}
}

void InputHandler::updateInputQueue(float deltaTime)
{
	std::deque<Input::InputTime>::iterator iter = inputQueue_.begin();
	while( iter != inputQueue_.end())
	{
		iter->second -= deltaTime;
		if (iter->second <= 0)
		{
			std::cout << "removed input: " << iter->first << "Side: "<< side_ << std::endl;
			inputQueue_.pop_front();
			iter = inputQueue_.begin();
		}
		else
		{
			iter++;
		}
	}
}

bool InputHandler::isSequenceInInputQueue(std::vector<uint8_t> neededInputs) const
{
	auto inputQueueIter = inputQueue_.begin();
	if (inputQueueIter != inputQueue_.end())
	{
		for (auto needInput = neededInputs.begin(); needInput != neededInputs.end() ; needInput++)
		{
			if (inputQueueIter == inputQueue_.end())
				return false;
			uint8_t returnVal = inputQueueIter->first & *needInput;
			if (returnVal)
			{
				inputQueueIter++;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void InputHandler::clearInputQueue()
{
	inputQueue_.clear();
}
