#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"
#include "EngineSettings.h"
#include "Hitbox.h"
#include "Attack.h"
#include "FighterStates/BaseFighterState.h"


//Hitbox structure used for all differrent types of hitboxs(hurtboxes, collisions, etc..) 
//Gamestate manager should be handling all collision detection.

typedef uint8_t InputKey;


struct AttackInput
{
	//user defined
	std::vector<InputKey> attackInput;
	int numInputs = 1;
	int attackIndex = 0;
	//struct private
	double dtBetweenAttacks; // IN MILLISECONDS
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

struct FighterStateData
{
	std::string animationName;
	std::vector<std::vector<Hitbox>> hitboxData;
};

struct AttackResources
{
	std::vector<Attack> attacks_;
	std::vector<AttackInput> inputs_;
};

class Fighter
{
public:
	Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterStateData crouchStatedata, FighterStateData jumpStateData, FighterStateData hitStateData, FighterStateData blockStateData, AttackResources attacks, FighterSide side = right);

	~Fighter();

	void onUpdate(float delta);

	void setPosition(glm::vec3 pos);

	void flipSide();

	bool onHit(Attack& attack); //returns bool if hit was successful 

	glm::vec3 getPosition() const;

	void setCurrentHitboxes(const std::vector<Hitbox>& hitboxes);

	//this will update currentXSpeed and adjust it depending on which way the fighter is facing.
	//positive speed value is walk forward, negative speed value is walk back
	void setXSpeed(float speedX);

	//Returns X speed in terms of what direction the fighter is facing. + value means heading toward where they're facing - value means heading away from where they're facing
	float getXSpeed();

	//this will update currentYSpeed and adjust it depending on which way the fighter is facing.
	//positive speed value is up, negative speed value is down
	void setYSpeed(float speedY);

	void handleWallCollision(bool collidedWithLeftWall);// called by gamestateManager when this fighter collides with a wall. argument is true when the fighter hits the left, false when they hit the right
	void handleFloorCollision();// called by gameStateManager when this fighter collides with the floor

	void updateTransform();

	void displaceFighter(float y, float z);

	void takeDamage(int damage);
public:
	AttackResources standingAttacks_;
	AttackResources crouchingAttacks_;
	AttackResources jumpingAttacks_;

	Entity* entity_;

	FighterSide side_;

	bool controllable_ = false;

	FighterState state_ = idle;

	BaseFighterState* newState_;

	double pastTime_ = 0;

	float baseSpeed_ = 5.0f;
	
	float terminalYSpeed_ = 0.25f;
	float currentYSpeed_ = 0.0f;
	float currentXSpeed_ = 0.0f;
	float gravity_ = -1.0f;
	float threshold;

	float deltaTime_;
	
	glm::vec2 currentMovement_;
	glm::vec2 lastMovement_;

	int numAttacks_ = 2;

	//set to -1 when not attacking
	//int currentAttack_ = -1;
	Attack* currentAttack_ = nullptr;

	double attackInputDelay_ = 70; //ms

	std::vector<AttackInput> attackInputs_;

	std::queue<int> attackBuffer_;

	float pushMagnitude_ = 0;

	int stunFrames_ = -1;

	int maxHealth_ = 100;

	bool flipSide_ = false;

	const double timeToClearBuffer_ = 5; //ms

	double clearAttackBufferTime_ = 0;

	//-----------------delete once new state machine is implemented--------------
	FighterStateData idleStateData_;
	FighterStateData walkingStateData_;
	FighterStateData crouchingStateData_;
	//---------------------end------------------------------------

	BaseFighterState* idleFighterState_ = nullptr;
	BaseFighterState* walkingFighterState_ = nullptr;
	BaseFighterState* jumpingFighterState_ = nullptr;
	BaseFighterState* jumpingAttackFighterState_ = nullptr;
	BaseFighterState* attackingFighterState_ = nullptr;
	BaseFighterState* crouchingFighterState_ = nullptr;
	BaseFighterState* hitFighterState_ = nullptr;
	BaseFighterState* blockedFighterState_ = nullptr;

	std::vector<Attack> attacks_;

	std::vector<Hitbox> defaultHitboxes_;//delete

	std::vector<Hitbox> currentHurtboxes_;
	std::vector<Hitbox> currentHitboxes_;
	std::vector<Hitbox> currentPushBoxes_;

	std::map<int, std::vector<int>>  cancelAttackMap_;

	InputHandler& inputHandler_;

	int health_ = 100;
private:

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	void handleStateTransition(BaseFighterState* transitionToState);

	//returns true if attack input is complete and populates attackIndex with correspoding attack
	bool checkAttackInput(int currentAttackInput, int& attackIndex);

private:

	float movedThisFrame_;
};

