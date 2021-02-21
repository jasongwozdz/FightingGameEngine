#include "DebugCamera.h"
#include <iostream>

DebugCamera::DebugCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDir) : 
	BaseCamera(pos, direction, upDir)
{
}

void DebugCamera::handleKeyPressed(Events::KeyPressedEvent& e)
{
	forwardHeld_ = e.KeyCode == 87; //w
	backwardHeld_ = e.KeyCode == 83; //s
	strafeLeftHeld_ = e.KeyCode == 65;//a
	strafeRightHeld_ = e.KeyCode == 68; //d
}

void DebugCamera::handleKeyReleased(Events::KeyReleasedEvent& e)
{
	forwardHeld_ = forwardHeld_ ^ (e.KeyCode == 87); //w
	backwardHeld_ = backwardHeld_ ^ (e.KeyCode == 83); //s
	strafeLeftHeld_ = strafeLeftHeld_ ^ (e.KeyCode == 65);//a
	strafeRightHeld_ = strafeRightHeld_ ^ (e.KeyCode == 68); //d
}

void DebugCamera::update(float deltaTime)
{
	if (forwardHeld_) moveForward(deltaTime);
	if (backwardHeld_) moveBackward(deltaTime);
	if (strafeLeftHeld_) strafeLeft(deltaTime);
	if (strafeRightHeld_) strafeRight(deltaTime);
}
