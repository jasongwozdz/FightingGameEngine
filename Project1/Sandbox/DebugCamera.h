#pragma once
#include "BaseCamera.h"
#include "Events.h"

class DebugCamera :
	public BaseCamera
{
public:
	DebugCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDir);

	void handleKeyPressed(Events::KeyPressedEvent& e);

	void handleKeyReleased(Events::KeyReleasedEvent& e);

	virtual void update(float deltaTime);

	bool forwardHeld_ = false;
	bool backwardHeld_ = false;
	bool strafeLeftHeld_ = false;
	bool strafeRightHeld_ = false;
};

