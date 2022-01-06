#include "Editor.h"

#include <iostream>

#include "../EntryPoint.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"

Application* createApplication()
{
	return new Editor();
}

void Editor::onStartup()
{
	addEventCallback(ENGINE_EVENT_CALLBACK(Editor::onEvent));
	setCursor(true);
	//Entity* entity = scene_->addEntity("GameObject");
	//entity->addComponent<Transform>(Transform(glm::vec3(0.0f, 0.0f, 2.0f)));
}

void Editor::onUpdate(float deltaTime)
{
	deltaTime_ = deltaTime;
	glm::vec3 origin = { 0, 0, 1 };
	debugManager_->drawLine(origin, origin + Transform::globalForward, { 0,0,255 });//forward blue
	debugManager_->drawLine(origin, origin + Transform::globalRight, { 255, 0, 0 });//right red
	debugManager_->drawLine(origin, origin + Transform::globalUp, { 0, 255,  0});//blue up
	adjustCameraRotation();
}

void Editor::adjustCameraRotation()
{
	static const glm::vec3 origin = { 0, 0, 1 };
	Camera& camera = scene_->getCurrentCamera();
	Transform& transform = camera.entity_->getComponent<Transform>();
	glm::vec3 desiredDirection = glm::normalize(origin - transform.position_);
	transform.lookAt(desiredDirection);
}

void Editor::moveCamera(Events::KeyEvent& keyEvent)
{
	static const float CameraSpeed = 1.0f;
	Camera& camera = scene_->getCurrentCamera();
	Transform& transform = camera.entity_->getComponent<Transform>();
	switch (keyEvent.KeyCode)
	{
		case GLFW_KEY_W:
		{
			transform.position_ += transform.forward() * CameraSpeed * deltaTime_;
			break;
		}
		case GLFW_KEY_S:
		{
			transform.position_ += transform.forward() * -CameraSpeed * deltaTime_;
			break;
		}
		case GLFW_KEY_SPACE:
		{
			transform.position_ += transform.up() * CameraSpeed * deltaTime_;
			break;
		}
	}
}

void Editor::onEvent(Events::Event& event)
{
	moveCamera(*(Events::KeyEvent*)&event);
}


