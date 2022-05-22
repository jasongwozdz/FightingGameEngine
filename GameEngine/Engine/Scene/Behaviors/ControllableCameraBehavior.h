#pragma once
#include "../Components/Behavior.h"
#include "../../EngineExport.h"

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
	float rotationSpeed_ = 0.5f;
	bool rightClickHeld = false;
	float horizontalAxis = 0;
	float verticalAxis = 0;
	glm::vec2 oldMousePos_;
	class Input* input_;
	
};

