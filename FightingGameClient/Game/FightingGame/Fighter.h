#pragma once
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "InputHandler.h"
#include "EngineSettings.h"
#include "Hitbox.h"
#include "Attack.h"


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

class Fighter
{
public:
	Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterSide side = right);

	void onUpdate(float delta);

	void setPosition(glm::vec3 pos);

	void flipSide();

	bool onHit(float pushMagnitude, int hitstunFrames, int blockStunFrames); //returns bool if hit was successful 

	void setOrKeepState(FighterState state);

	glm::vec3 getPosition() const;

	void setCurrentHitboxes(const std::vector<Hitbox>& hitboxes);

public:
	Entity* entity_;

	FighterSide side_;

	glm::vec3 gamePos_;
	
	bool controllable_ = false;

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

	FighterStateData idleStateData_;
	FighterStateData walkingStateData_;
	std::vector<Attack> attacks_;

	std::vector<Hitbox> defaultHitboxes_;

	std::vector<Hitbox> currentHurtboxes_;
	std::vector<Hitbox> currentHitboxes_;
	std::vector<Hitbox> currentPushBoxes_;

	std::map<int, std::vector<int>>  cancelAttackMap_;

private:

	void updateTransform();

	void enterState(FighterState state);

	void processInput();

	void handleState();

	void handleMove();

	//returns true if attack input is complete and populates attackIndex with correspoding attack
	bool checkAttackInput(int currentAttackInput, int& attackIndex);

private:

	float movedThisFrame_;

	InputHandler& inputHandler_;

};

