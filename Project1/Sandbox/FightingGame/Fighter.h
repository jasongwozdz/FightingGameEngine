#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"
#include "EngineSettings.h"


//Hitbox structure used for all differrent types of hitboxs(hurtboxes, collisions, etc..) 
//Gamestate manager should be handling all collision detection.

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
	std::vector<Hitbox> children;
	//TODO: Hitbox* children_; Implement hierarchical hitbox system.  
};

struct Hitboxes
{

	std::vector<Hitbox> children;
};

struct Attack
{
	Attack() = default;

	Attack& operator=(const Attack& other)
	{
		startupFrames = (other.startupFrames);
		activeFrames = (other.activeFrames);
		recoveryFrames = (other.recoveryFrames);
		hurtbox_ = (other.hurtbox_);
		animationIndex_ = (other.animationIndex_);
		blockstunFrames = (other.blockstunFrames);
		hitstunFrames = (other.hitstunFrames);
		hurtboxPos = new glm::vec3[activeFrames];
		hurtboxWidthHeight = new glm::vec2[activeFrames];
		memcpy(hurtboxPos, other.hurtboxPos, sizeof(glm::vec3) * activeFrames);
		memcpy(hurtboxWidthHeight, other.hurtboxWidthHeight, sizeof(glm::vec2) * activeFrames);
		damage = other.damage;
		return *this;
	}

	Attack& operator=(Attack&& other)
	{
		startupFrames = (other.startupFrames);
		activeFrames = (other.activeFrames);
		recoveryFrames = (other.recoveryFrames);
		hurtbox_ = (other.hurtbox_);
		animationIndex_ = (other.animationIndex_);
		blockstunFrames = (other.blockstunFrames);
		hitstunFrames = (other.hitstunFrames);
		hurtboxPos = other.hurtboxPos;
		hurtboxWidthHeight = other.hurtboxWidthHeight;
		other.hurtboxPos = nullptr;
		other.hurtboxWidthHeight = nullptr;
		hitPushMag = other.hitPushMag; 
		blockPushMag = other.blockPushMag;
		damage = other.damage;
		return *this;
	}

	Attack(int startup, int active, int recovery, glm::vec2* hurtboxWidthHeightIn, glm::vec3* hurtboxPosIn, int animationIndexIn, Hitbox hurtboxIn, int blockstun, int hitstun, float pushMag, int dam) :
		startupFrames(startup), activeFrames(active), recoveryFrames(recovery),hurtboxWidthHeight(hurtboxWidthHeightIn), hurtboxPos(hurtboxPosIn), animationIndex_(animationIndexIn), hurtbox_(hurtboxIn), blockstunFrames(blockstun), hitstunFrames(hitstun), hitPushMag(pushMag), damage(dam)
	{}

	Attack(const Attack& other) :
		startupFrames(other.startupFrames),
		activeFrames(other.activeFrames),
		recoveryFrames(other.recoveryFrames),
		hurtbox_(other.hurtbox_),
		animationIndex_(other.animationIndex_),
		blockstunFrames(other.blockstunFrames),
		hitstunFrames(other.hitstunFrames)
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
		animationIndex_(other.animationIndex_),
		blockstunFrames(other.blockstunFrames),
		hitstunFrames(other.hitstunFrames),
		damage(other.damage)
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

	int blockstunFrames;
	int hitstunFrames;

	glm::vec2* hurtboxWidthHeight; //array the size of active frames
	glm::vec3* hurtboxPos; //array the size of active frames

	int animationIndex_;
	Hitbox hurtbox_;

	float hitPushMag; //magnitude of push when hit
	float blockPushMag; //magnitude of push when blocked

	bool handled_ = false;

	int damage = 0;
};

struct AttackInput
{
	//user defined
	std::vector<uint8_t> attackInput;
	int numInputs = 1;
	double dtBetweenAttacks; // IN MILLISECONDS
	int attackIndex = 0;
	//struct private
	double lastCheckTime = 0;
	float currentInput = 0;
};

enum FighterState
{
	idle,
	walking,
	jumping,
	attacking,
	hitstun,
	blocking
};

enum FighterSide
{
	left = 0,
	right = 1
};

class Fighter
{
public:
	Fighter(Entity* entity, InputHandler& inputHandler, FighterSide side = right);

	void onUpdate(float delta);

	void setPosition(glm::vec3 pos);

	void flipSide();

	FighterSide side_;
	
	bool controllable_ = false;

	Entity* entity_;

	FighterState state_ = idle;

	double pastTime_ = 0;

	float baseSpeed_ = 5.0f;
	
	float speed_ = baseSpeed_;
	float terminalYSpeed_ = 0.25f;
	float currentYspeed_ = 0.0f;
	float currentXspeed_ = 0.0f;
	float gravity_ = -1.0f;
	float threshold;

	float deltaTime_;
	
	glm::vec2 currentMovement_;

	int numAttacks_ = 2;

	//set to -1 when not attacking
	int currentAttack_ = -1;

	double attackInputDelay_ = 70; //ms

	std::vector<AttackInput> attackInputs_;

	std::queue<int> attackBuffer_;

	float pushMagnitude_ = 0;

	int stunFrames_ = -1;

	int maxHealth_ = 100;

	bool flipSide_ = false;

	const double timeToClearBuffer_ = 5; //ms
	double clearAttackBufferTime_ = 0;

	bool onHit(float pushMagnitude, int hitstunFrames, int blockStunFrames); //returns bool if hit was successful 

	void setOrKeepState(FighterState state);

	glm::vec3 getPosition();

private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	//returns true if attack input is complete and populates attackIndex with correspoding attack
	bool checkAttackInput(int currentAttackInput, int& attackIndex);

	float movedThisFrame_;

	InputHandler& inputHandler_;

};

