#include "CameraController.h"
#include <iostream>

CameraController::CameraController(BaseCamera& camera) :
	camera_(camera)
{}

void CameraController::handleKeyPressed(Events::KeyPressedEvent& e)
{
	if (e.KeyCode == 87)
	{
		forwardHeld_ = true;
		std::cout << "held" << std::endl;
	}
	else if (e.KeyCode == 83)
	{
		backwardHeld_ = true;
	}
	else if (e.KeyCode == 65)//a
	{
		strafeLeftHeld_ = true;
	}
	else if (e.KeyCode == 68)
	{
		strafeRightHeld_ = true;
	}

	//forwardHeld_ = e.KeyCode == 87; //w
	//backwardHeld_ = e.KeyCode == 83; //s
	//strafeLeftHeld_ = e.KeyCode == 65;//a
	//strafeRightHeld_ = e.KeyCode == 68; //d
}

void CameraController::handleKeyReleased(Events::KeyReleasedEvent& e)
{
	if (e.KeyCode == 87)
	{
		forwardHeld_ = false;
		std::cout << "released" << std::endl;
	}
	else if (e.KeyCode == 83)
	{
		backwardHeld_ = false;
	}
	else if (e.KeyCode == 65)//a
	{
		strafeLeftHeld_ = false;
	}
	else if (e.KeyCode == 68)
	{
		strafeRightHeld_ = false;
	}
	//forwardHeld_ = forwardHeld_ ^ (e.KeyCode == 87); //w
	//backwardHeld_ = backwardHeld_ ^ (e.KeyCode == 83); //s
	//strafeLeftHeld_ = strafeLeftHeld_ ^ (e.KeyCode == 65);//a
	//strafeRightHeld_ = strafeRightHeld_ ^ (e.KeyCode == 68); //d
}

void CameraController::handleMouseMoved(Events::MouseMoveEvent& e)
{
	camera_.updateMouse({ e.mouseX, e.mouseY });
}

void CameraController::onUpdate(float deltaTime)
{
	//if (forwardHeld_) 
	//	camera_.moveForward(deltaTime);
	//if (backwardHeld_) 
	//	camera_.moveBackward(deltaTime);
	//if (strafeLeftHeld_) 
	//	camera_.strafeLeft(deltaTime);
	//if (strafeRightHeld_) 
	//	camera_.strafeRight(deltaTime);
}
