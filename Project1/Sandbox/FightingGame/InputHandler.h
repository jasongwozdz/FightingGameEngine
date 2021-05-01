#pragma once
#include "Events.h"
#include <glm/glm.hpp>

namespace Input
{
	enum Side
	{
		left,
		right
	};

	enum AttackMap
	{
		light = 1, 
		medium = 2, 
		strong = 4, 
		ultra = 8,
		notLight = 14,
		notMedium = 13,
		notStrong = 11,
		notUltra = 7
	};
}

class InputHandler
{
public:

	void handleInputPressed(Events::KeyPressedEvent& e);

	void handleInputReleased(Events::KeyReleasedEvent& e);
	
	glm::vec2 currentInput_;
	int currentAttackInput_ = 0;

	Input::Side side_ = Input::Side::left;
};
