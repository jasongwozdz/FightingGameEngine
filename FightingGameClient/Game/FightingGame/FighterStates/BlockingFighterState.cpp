#include "BlockingFighterState.h"

#include <iostream>

#include "../Fighter/Fighter.h"


BlockingFighterState::BlockingFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData) :
	BaseFighterState(animationName, hitboxData)
{}

BaseFighterState* BlockingFighterState::update(Fighter* fighter)
{
	updateCurrentHitboxes(fighter);
	fighter->setXSpeed(movePerFrame_);
	if (gravity_ == 0)//if fighter was not hit while in the air then do normal blockstun processing
	{
		//go back to idleState once we serve our blockstun time
		if (numberOfBlockstunFrames_++ > hitByAttack_->blockstunFrames)
		{
			return fighter->idleFighterState_;
		}
	}
	else //if fighter was hit in the air then they are in hitstun until they hit the ground
	{
		fighter->velocityWorldSpace_ += gravity_;
		//fighter->currentYSpeed_ += gravity_;
	}
	return nullptr;
}

void BlockingFighterState::enterState(Fighter* fighter)
{
	std::cout << "Enter Blocking state" << std::endl;
	gravity_ = 0;//reset gravity
	currentBlockstunFrame_ = 0;//reset hitstun frame

	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	assert(hitByAttack_);
	distanceToMove_ = hitByAttack_->blockPushMag;
	movePerFrame_ = (distanceToMove_ / hitByAttack_->blockstunFrames);
	//check if fighter is currently in the air
	if (inAir_)
	{
		//fighter is currently in the air so set gravity
		gravity_ = DEFAULT_GRAVITY;
	}
}

BaseFighterState* BlockingFighterState::handleMovementInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* BlockingFighterState::handleAttackInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* BlockingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	currentBlockstunFrame_ = 0;//reset hitstun frame
	hitByAttack_ = attack;
	if(inAir_)
	{
		fighter->setYSpeed(0);//reset y speed when blocking in air
	}
	return nullptr;
}

BaseFighterState* BlockingFighterState::handleWallCollision(Fighter* fighter, bool collidedWithLeftSide)
{
	return nullptr;
}

BaseFighterState* BlockingFighterState::handleFloorCollision(Fighter* fighter)
{
	gravity_ = 0;
	return nullptr;
}
