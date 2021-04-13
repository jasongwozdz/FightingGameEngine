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
	Fighter(Entity& entity, Transform& transform, Animator& animator, InputHandler& inputHandler);
	void onUpdate(float delta);
	
private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	void setOrKeepState(FighterState state);

	void enterState();

	void jumping();

	float speed_ = 25.0f;

	float movedThisFrame_;

	Entity& entity_;

	Animator& animator_;

	Transform& transform_;

	InputHandler& inputHandler_;

	FighterState state_ = idle;

	glm::vec2 currentMovement_;

	bool controllable_ = true;
};

