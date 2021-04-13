#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Transform.h"

//class Camera
//{
//public:
//	Camera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDirection);
//	
//	~Camera();
//
//	glm::mat4 getView(Transform* transform); //optional transform argument to extract position from transform component
//
//	void setPosition(glm::vec3 pos);
//	
//	void updateMouse(glm::vec2 newMousePosition);
//	
//	void moveForward(float time);
//
//	void moveBackward(float time);
//
//	void strafeLeft(float time);
//
//	void strafeRight(float time);
//
//	void setLookDirection(glm::vec3 direction);
//
//	void setOldMousePosition(glm::vec2 oldMouse);
//
//private:
//
//	glm::vec3 viewDirection_;
//
//	glm::vec3 position_;
//
//	glm::vec3 upDirection_ = glm::vec3(0.0f, 0.0f, 1.0f);
//
//	float rotationSpeed_ = 0.005f;
//	
//	float cameraSpeed_ = 0.005f;
//};

