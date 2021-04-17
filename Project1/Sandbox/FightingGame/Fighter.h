#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"

enum FighterState
{
	idle,
	walking,
	jumping,
	attacking
};

enum FighterStartingSide
{
	left,
	right
};

class Fighter
{
public:
	Fighter(Entity& entity, InputHandler& inputHandler);

	void onUpdate(float delta);

	void setPosition(glm::vec3 pos);

	void flipSide();
	
	bool controllable_ = false;

	Entity& entity_;
private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	void setOrKeepState(FighterState state);

	void enterState();

	void jumping();

	float speed_ = 0.05f;

	float movedThisFrame_;


	InputHandler& inputHandler_;

	FighterState state_ = idle;

	glm::vec2 currentMovement_;
};

