#include "CameraController.h"
#include <iostream>
#include "GLFW/glfw3.h"

CameraController::CameraController(BaseCamera& camera) :
	camera_(camera)
{}

void CameraController::handleKeyPressed(Events::KeyPressedEvent& e)
{
	if (controllable_)
	{
		if (e.KeyCode == GLFW_KEY_I)
		{
			forwardHeld_ = true;
		}
		else if (e.KeyCode == GLFW_KEY_K)
		{
			backwardHeld_ = true;
		}
		else if (e.KeyCode == GLFW_KEY_J)//a
		{
			strafeLeftHeld_ = true;
		}
		else if (e.KeyCode == GLFW_KEY_L)
		{
			strafeRightHeld_ = true;
		}
	}
}

void CameraController::handleKeyReleased(Events::KeyReleasedEvent& e)
{
	if (controllable_)
	{
		if (e.KeyCode == GLFW_KEY_I)
		{
			forwardHeld_ = false;
		}
		else if (e.KeyCode == GLFW_KEY_K)
		{
			backwardHeld_ = false;
		}
		else if (e.KeyCode == GLFW_KEY_J)//a
		{
			strafeLeftHeld_ = false;
		}
		else if (e.KeyCode == GLFW_KEY_L)
		{
			strafeRightHeld_ = false;
		}
	}
}

void CameraController::handleMouseMoved(Events::MouseMoveEvent& e)
{
	if (controllable_)
	{
		camera_.updateMouse({ e.mouseX, e.mouseY });
	}
}

void CameraController::onUpdate(float deltaTime)
{
	if (forwardHeld_) 
		camera_.moveForward(deltaTime);
	if (backwardHeld_) 
		camera_.moveBackward(deltaTime);
	if (strafeLeftHeld_) 
		camera_.strafeLeft(deltaTime);
	if (strafeRightHeld_) 
		camera_.strafeRight(deltaTime);
}
