#pragma once
#include "../Components/Behavior.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API ControllableCameraBehavior : public BehaviorImplementationBase
{
public:

	ControllableCameraBehavior(Entity* entity);
	virtual void update() override;

private:
	void handleKeyInput();
	void handleMouseInput();

private:
	float cameraSpeed_ = 0.05f;
	float rotationSpeed_ = 1.0f;
	bool rightClickHeld = false;
	float horizontalAxis = 0;
	float verticalAxis = 0;
	glm::vec2 oldMousePos_;
	class Input* input_;
	
};

