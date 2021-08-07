#pragma once
#include <map>
#include "Events.h"
#include <glm/glm.hpp>

namespace Input
{

	enum Side
	{
		leftSide,
		rightSide
	};

	enum Action
	{
		nothing = 0,
		left = 1,
		right = 2,
		down = 4,
		up  = 8,
		light = 16, 
		medium = 32, 
		strong = 64,
		ultra = 128,
	};
}

class InputHandler
{
public:
	InputHandler() = default;

	void handleInputPressed(Events::KeyPressedEvent& e);

	void handleInputReleased(Events::KeyReleasedEvent& e);

	bool isInputCurrentlyPressed(Input::Action input);
	
	glm::vec2 currentMovementInput_;
	uint8_t currentAttackInput_ = 0;

	Input::Side side_ = Input::Side::leftSide;

private:
	std::map<Input::Action, unsigned int> actionKeyMap;
};
