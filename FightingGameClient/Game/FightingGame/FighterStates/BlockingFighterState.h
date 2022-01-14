#pragma once
#include "BaseFighterState.h"

class BlockingFighterState :
	public BaseFighterState
{
public:
	BlockingFighterState(std::string animationName, std::vector<FrameInfo> frameData);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, Attack* attack) override;
	BaseFighterState* handleWallCollision(Fighter* fighter, bool collidedWithLeftSide) override;
	BaseFighterState* handleFloorCollision(Fighter* fighter) override;
public:
	Attack* hitByAttack_;
	bool inAir_ = false;
private:
	float distanceToMove_ = 0;
	float movePerFrame_ = 0;
	float numberOfBlockstunFrames_ = 0;
	float currentBlockstunFrame_ = 0;
	float gravity_= 0;
	int comboCount_ = 0;
};

