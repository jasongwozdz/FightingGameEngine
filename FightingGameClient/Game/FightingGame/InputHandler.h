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
		left = 1, // 1 << 0
		right = 2, // 1 << 1
		down = 4, // 1 << 2
		up  = 8, // 1 << 3
		light = 16, // 1 << 4
		medium = 32, // 1 << 5
		strong = 64,// 1 << 6
		ultra = 128, // 1 << 7
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
