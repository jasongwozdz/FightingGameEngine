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
}

class InputHandler
{
public:

	void handleInputPressed(Events::KeyPressedEvent& e);

	void handleInputReleased(Events::KeyReleasedEvent& e);
	
	glm::vec2& getInput();

	glm::vec2 currentInput_;

	Input::Side side_ = Input::Side::left;
};
