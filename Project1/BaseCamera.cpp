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

void BaseCamera::moveForward(uint64_t time) 
{
	position += time * cameraSpeed * viewDirection;
}

void BaseCamera::moveBackward(uint64_t time) 
{
	position -= time * cameraSpeed * viewDirection;
}

void BaseCamera::strafeLeft(uint64_t time) 
{
	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
	position -= time * cameraSpeed * strafeDirection;
}

void BaseCamera::strafeRight(uint64_t time) 
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
	//glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 3.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	return view;
}

