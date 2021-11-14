#pragma once
#include "Events.h"
#include "BaseCamera.h"

class CameraController
{
public:
	CameraController();
	CameraController(BaseCamera* camera);

	void handleKeyReleased(Events::KeyReleasedEvent& e);
	void handleKeyPressed(Events::KeyPressedEvent& e);
	void handleMouseMoved(Events::MouseMoveEvent& e);

	void setCurrentCamera(BaseCamera* camera);
	void onUpdate(float deltaTime);

	bool controllable_ = true;

private:
	BaseCamera* camera_;

	bool forwardHeld_ = false;
	bool backwardHeld_ = false;
	bool strafeLeftHeld_ = false;
	bool strafeRightHeld_ = false;
};

