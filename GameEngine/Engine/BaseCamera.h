#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <queue>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

struct ENGINE_API ScrollingBuffer
{
	int maxSize_;
	int offset_;
	std::vector<glm::vec2> data;

};

class ENGINE_API BaseCamera
{
public:
	BaseCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDirection, float fov);
	
	~BaseCamera();

	void updateMouse(glm::vec2 newMousePosition);
	
	void moveForward(float time);

	void moveBackward(float time);

	void strafeLeft(float time);

	void strafeRight(float time);

	void setFov(float newFovInDegrees);

	void setOldMousePosition(glm::vec2 oldMouse);

	void resetProjectionMatrix(const float width, const float height);

	glm::mat4 const getView();

	virtual void update(float deltaTime);

	glm::mat4 projectionMatrix;
	glm::vec3 viewDirection;
	glm::vec3 position;
	glm::vec3 upDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec2 oldMousePosition;

	std::vector<float> times_;
	std::vector<float> viewDirectionsX_;
	std::vector<float> viewDirectionsY_;
	std::vector<float> viewDirectionsZ_;

	std::vector<float> posX_;
	std::vector<float> posY_;
	std::vector<float> posZ_;
	int numberOfDataPoints_ = 1000;

	bool drawDebug_ = false;
	
	float rotationSpeed = 0.005f;
	
	float cameraSpeed = 0.005f;

	int cameraIndex_ = -1;//set when the camera is added to the scene
private:
	float fovInDegrees;
};

