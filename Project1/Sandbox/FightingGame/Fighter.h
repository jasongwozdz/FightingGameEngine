#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"

struct Hitbox
{
	Hitbox() = default;

	Hitbox(float width, float height, glm::vec3 pos) :
		width_(width), height_(height), pos_(pos), hitboxEnt_(nullptr)
	{}

	float width_;
	float height_;
	glm::vec3 pos_;
	Entity* hitboxEnt_;
};

struct Attack
{
	Attack() = default;

	Attack(int startup, int active, int recovery, glm::vec2* hurtboxWidthHeightIn, glm::vec3* hurtboxPosIn, int animationIndexIn, Hitbox hurtboxIn) :
		startupFrames(startup), activeFrames(active), recoveryFrames(recovery),hurtboxWidthHeight(hurtboxWidthHeightIn), hurtboxPos(hurtboxPosIn), animationIndex_(animationIndexIn), hurtbox_(hurtboxIn)
	{}

	Attack(const Attack& other) :
		startupFrames(other.startupFrames),
		activeFrames(other.activeFrames),
		recoveryFrames(other.recoveryFrames),
		hurtbox_(other.hurtbox_),
		animationIndex_(other.animationIndex_)
	{
		hurtboxPos = new glm::vec3[activeFrames];
		hurtboxWidthHeight = new glm::vec2[activeFrames];
		memcpy(hurtboxPos, other.hurtboxPos, sizeof(glm::vec3) * activeFrames);
		memcpy(hurtboxWidthHeight, other.hurtboxWidthHeight, sizeof(glm::vec2) * activeFrames);
	}

	Attack(Attack&& other)
		:startupFrames(other.startupFrames),
		activeFrames(other.activeFrames),
		recoveryFrames(other.recoveryFrames),
		hurtbox_(other.hurtbox_),
		animationIndex_(other.animationIndex_)
	{
		hurtboxPos = other.hurtboxPos;
		hurtboxWidthHeight = other.hurtboxWidthHeight;
		other.hurtboxWidthHeight = nullptr;
		other.hurtboxPos = nullptr;
	}

	~Attack()
	{
		delete[] hurtboxWidthHeight;
		delete[] hurtboxPos;
	}

	int currentFrame = -1;

	int startupFrames; //no hurtox active
	int activeFrames; //hurtbox active
	int recoveryFrames; //no hurtbox active

	glm::vec2* hurtboxWidthHeight; //array the size of active frames
	glm::vec3* hurtboxPos; //array the size of active frames

	int animationIndex_;
	Hitbox hurtbox_;
};

struct AttackInput
{
	int numInputs = 1;
	std::vector<int> attackInput;
	std::vector<glm::vec2> movementInput;
	int attackIndex;
	int currentInput = 0;
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

	int numAttacks_ = 1;

	//set to -1 when not attacking
	int currentAttack_ = -1;

	std::vector<AttackInput> attackInputs_;

private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	void setOrKeepState(FighterState state);

	void enterState();

	//returns true if attack input is complete and populates attackIndex with correspoding attack
	bool checkAttackInput(glm::vec2& currentMovement, int currentAttackInput, int& attackIndex);

	float movedThisFrame_;

	InputHandler& inputHandler_;
};

