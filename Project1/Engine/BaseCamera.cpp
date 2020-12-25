#include "BaseCamera.h"

BaseCamera::BaseCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDir)
{
	position = pos;
	viewDirection = direction;
	upDirection = upDir;
}

BaseCamera::~BaseCamera() {};

void BaseCamera::setPosition(glm::vec3 pos) 
{
	position = pos;
}

void BaseCamera::updateMouse(glm::vec2 newMousePosition) 
{
	glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

	//rotate around up axis by delta x 
	viewDirection = glm::mat3(glm::rotate(-mouseDelta.x * rotationSpeed, upDirection)) * viewDirection;
	
	//get new axis to rotate around for y direction
	glm::vec3 toRotateAround = glm::cross(viewDirection, upDirection);

	//rotate around y axis by delta y
	viewDirection = glm::mat3(glm::rotate(-mouseDelta.y * rotationSpeed, toRotateAround)) * viewDirection; 
	oldMousePosition = newMousePosition;
}

void BaseCamera::moveForward(float time) 
{
	position += time * cameraSpeed * viewDirection;
}

void BaseCamera::moveBackward(float time) 
{
	position -= time * cameraSpeed * viewDirection;
}

void BaseCamera::strafeLeft(float time) 
{
	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
	position -= time * cameraSpeed * strafeDirection;
}

void BaseCamera::strafeRight(float time) 
{
	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
	position += time * cameraSpeed * strafeDirection;
}

void BaseCamera::setLookDirection(glm::vec3 direction) 
{
	viewDirection = direction;
}

glm::mat4 BaseCamera::getView() 
{
	glm::mat4 view = glm::lookAt(position, position + viewDirection, upDirection);
	return view;
}

