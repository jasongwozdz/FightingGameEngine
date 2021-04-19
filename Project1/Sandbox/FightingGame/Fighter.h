#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"

struct Hitbox
{
	Hitbox(float width, float height, glm::vec3 pos) :
		width_(width), height_(height), pos_(pos), hitboxEnt_(nullptr){}

	float width_;
	float height_;
	glm::vec3 pos_;
	Entity* hitboxEnt_;
};

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
	Fighter(Entity* entity, InputHandler& inputHandler);

	void onUpdate(float delta);

	void setPosition(glm::vec3 pos);

	void flipSide();
	
	bool controllable_ = false;

	Entity* entity_;

	FighterState state_ = idle;

	float baseSpeed_ = 0.05f;
	
	float speed_ = baseSpeed_;

	float deltaTime_;
	
	glm::vec2 currentMovement_;
private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	void setOrKeepState(FighterState state);

	void enterState();

	void jumping();

	float movedThisFrame_;

	InputHandler& inputHandler_;
};

