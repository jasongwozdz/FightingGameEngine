#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"
#include "EngineSettings.h"
#include "Hitbox.h"


//Hitbox structure used for all differrent types of hitboxs(hurtboxes, collisions, etc..) 
//Gamestate manager should be handling all collision detection.

typedef uint8_t InputKey;


struct Attack
{
	Attack() = default;

	Attack(int startup, int active, int recovery, std::vector<glm::vec2> hurtboxWidthHeightIn, std::vector<glm::vec2> hurtboxPosIn, int animationIndexIn, Hitbox hurtboxIn, int blockstun, int hitstun, float pushMag, int dam) :
		startupFrames(startup), activeFrames(active), recoveryFrames(recovery),hurtboxWidthHeight(hurtboxWidthHeightIn), hurtboxPos(hurtboxPosIn), animationIndex_(animationIndexIn), hurtbox_(hurtboxIn), blockstunFrames(blockstun), hitstunFrames(hitstun), hitPushMag(pushMag), damage(dam)
	{}

	int currentFrame = -1;

	int startupFrames; //no hurtox active
	int activeFrames; //hurtbox active
	int recoveryFrames; //no hurtbox active

	int blockstunFrames;
	int hitstunFrames;

	//glm::vec2* hurtboxWidthHeight; //array the size of active frames
	std::vector<glm::vec2> hurtboxWidthHeight; //array the size of active frames
	//glm::vec2* hurtboxPos; //array the size of active frames
	std::vector<glm::vec2> hurtboxPos; //array the size of active frames

	int animationIndex_;
	Hitbox hurtbox_;

	float hitPushMag; //magnitude of push when hit
	float blockPushMag; //magnitude of push when blocked

	bool handled_ = false;

	int damage = 0;

	bool jumpAttack = false;
};

struct AttackInput
{
	//user defined
	std::vector<InputKey> attackInput;
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
	walkingForward,
	walkingBackward,
	jumping,
	attacking,
	crouching,
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

	glm::vec3 gamePos_;
	
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
	glm::vec2 lastMovement_;

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

	std::vector<Attack> attacks_;

	bool onHit(float pushMagnitude, int hitstunFrames, int blockStunFrames); //returns bool if hit was successful 

	void setOrKeepState(FighterState state);

	glm::vec3 getPosition() const;


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

