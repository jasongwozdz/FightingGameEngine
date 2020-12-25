#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class BaseCamera
{
private:
	glm::vec3 viewDirection;
	
	glm::vec3 position;
	
	glm::vec3 upDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	
	glm::vec2 oldMousePosition;
	
	float rotationSpeed = 0.005f;
	
	float cameraSpeed = 0.005f;
public:
	BaseCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDirection);
	
	~BaseCamera();

	void setPosition(glm::vec3 pos);
	
	void updateMouse(glm::vec2 newMousePosition);
	
	void moveForward(float time);

	void moveBackward(float time);

	void strafeLeft(float time);

	void strafeRight(float time);

	void setLookDirection(glm::vec3 direction);

	glm::mat4 getView();
};

