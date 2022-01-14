#pragma once
#include <vector>

#include "Scene/Components/Collider.h"
#include "../FighterSandbox/AttackTypes.h"

//Forward declarations
class Fighter;
struct AttackResources;
struct Attack;

class BaseFighterState
{
public:
	~BaseFighterState() = default;

	virtual BaseFighterState* update(Fighter* fighter) = 0;
	virtual void enterState(Fighter* fighter) = 0;
	virtual BaseFighterState* handleMovementInput(Fighter* fighter) = 0;
	virtual BaseFighterState* handleAttackInput(Fighter* fighter) = 0;
	virtual BaseFighterState* onHit(Fighter* fighter, Attack* attack) = 0;
	virtual BaseFighterState* handleWallCollision(Fighter* fighter, bool collidedWithLeftSide) { return nullptr; }//OPTIONAL: called by gamestate manager when a wall is collided with.  Bool argument will be true if collided with left side of the arena, false if collided with right side of the arena
	virtual BaseFighterState* handleFloorCollision(Fighter* fighter) { return nullptr; }//OPTIONAL: called by gamestate manager when colliding with the ground

protected:
	BaseFighterState(std::string animationName, std::vector<FrameInfo> hitboxData) :
		animationName_(animationName),
		frameData_(hitboxData),
		currentFrame_(0)
	{};

	void updateCurrentHitboxes(Fighter* fighter);
	class AttackBase* checkAttackInputsNew(Fighter* fighter, AttackResources& attacks);
	bool isFighterHoldingBack(Fighter* fighter);
protected:
	std::string animationName_;
	//std::vector<std::vector<BoxCollider>> hitboxData_;
	std::vector<FrameInfo> frameData_;
	int currentFrame_;
};

