#pragma once
#include "BaseFighterState.h"
#include "../FighterSandbox/HitHandler.h"

class HitFighterState :
	public BaseFighterState
{
public:
	HitFighterState(std::string animationName, std::vector<FrameInfo> hitboxData);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, OnHitType attack) override;
	BaseFighterState* handleWallCollision(Fighter* fighter, bool collidedWithLeftSide) override;
	BaseFighterState* handleFloorCollision(Fighter* fighter) override;
public:
	HitEffect hitByEffect_;
	bool inAir_ = false;
private:
	HitHandler hitHandler_;
	float distanceToMove_ = 0;
	float movePerFrame_ = 0;
	float numberOfHitstunFrames_ = 0;
	float currentHitstunFrame_ = 0;
	float gravity_= 0;
	int comboCount_ = 1;
};

