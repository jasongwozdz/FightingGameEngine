#include "Camera.h"

//Camera::Camera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDir):
//	position_(pos),
//	viewDirection_(direction),
//	upDirection_(upDir)
//{
//}
//
//void Camera::setPosition(glm::vec3 pos) 
//{
//	position = pos;
//}
//
//void Camera::setOldMousePosition(glm::vec2 oldMouse)
//{
//	oldMousePosition = oldMouse;
//}
//
//void Camera::updateMouse(glm::vec2 newMousePosition) 
//{
//	glm::vec2 mouseDelta = newMousePosition - oldMousePosition;
//
//	//rotate around up axis by delta x 
//	viewDirection = glm::mat3(glm::rotate(-mouseDelta.x * rotationSpeed, upDirection)) * viewDirection;
//	
//	//get new axis to rotate around for y direction
//	glm::vec3 toRotateAround = glm::cross(viewDirection, upDirection);
//
//	//rotate around y axis by delta y
//	viewDirection = glm::mat3(glm::rotate(-mouseDelta.y * rotationSpeed, toRotateAround)) * viewDirection; 
//	oldMousePosition = newMousePosition;
//}
//
//void Camera::moveForward(float time) 
//{
//	position += time * cameraSpeed * viewDirection;
//}
//
//void Camera::moveBackward(float time) 
//{
//	position -= time * cameraSpeed * viewDirection;
//}
//
//void Camera::strafeLeft(float time) 
//{
//	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
//	position -= time * cameraSpeed * strafeDirection;
//}
//
//void Camera::strafeRight(float time) 
//{
//	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
//	position += time * cameraSpeed * strafeDirection;
//}
//
//void Camera::setLookDirection(glm::vec3 direction) 
//{
//	viewDirection = direction;
//}
//glm::mat4 Camera::getView(Transform* transform) 
//{
//	glm::vec3 pos = position_;
//	if (transform)
//		pos = transform->getPosition();
//	glm::mat4 view = glm::lookAt(pos, pos + viewDirection_, upDirection_);
//	return view;
//}
