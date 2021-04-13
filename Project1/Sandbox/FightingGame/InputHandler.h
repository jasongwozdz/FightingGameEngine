#pragma once
#include "Events.h"
#include <glm/glm.hpp>

class InputHandler
{
public:
	void handleInputPressed(Events::KeyPressedEvent& e);

	void handleInputReleased(Events::KeyReleasedEvent& e);
	
	glm::vec2& getInput();

	glm::vec2 currentInput_;
};
