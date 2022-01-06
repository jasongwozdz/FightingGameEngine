#include <iostream>
#include "HitFighterState.h"
#include "../Fighter/Attack.h"
#include "../Fighter/Fighter.h"
#include "../GameStateManager.h"
#include "Renderer/UIInterface.h"

HitFighterState::HitFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData) :
	BaseFighterState(animationName, hitboxData)
{}

BaseFighterState* HitFighterState::update(Fighter* fighter)
{
	updateCurrentHitboxes(fighter);
	std::cout << currentHitstunFrame_ << std::endl;
	if (gravity_ == 0)//if fighter was not hit while in the air then do normal hitstun processing
	{
		fighter->setXSpeed(movePerFrame_);
		//go back to idleState once we server our hitstun time
		if (currentHitstunFrame_++ > hitByAttack_->hitstunFrames)
		{
			return fighter->idleFighterState_;
		}
	}
	else //if fighter was hit in the air then they are in hitstun until they hit the ground
	{
		fighter->velocityWorldSpace_ += gravity_;
		//fighter->currentYSpeed_ += gravity_;
	}
	UI::UIInterface::getSingletonPtr()->addTextToTransparentBackground(std::to_string(comboCount_), { 0, 500 }, { 255, 0, 0, 100 }, 10);
	return nullptr;
}

void HitFighterState::enterState(Fighter* fighter)
{
	std::cout << "Enter Hit state" << std::endl;
	gravity_ = 0;//reset gravity
	currentHitstunFrame_ = 0;//reset hitstun frame
	comboCount_ = 1;

	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	assert(hitByAttack_);

	distanceToMove_ = hitByAttack_->hitPushMag;
	movePerFrame_ = (distanceToMove_ / hitByAttack_->hitstunFrames);
	//check if fighter is currently in the air
	if (inAir_)
	{
		fighter->setYSpeed(10);
		//fighter is currently in the air so set gravity
		gravity_ = DEFAULT_GRAVITY;
	}
}
BaseFighterState* HitFighterState::handleMovementInput(Fighter* fighter)
{
	return nullptr;
}
BaseFighterState* HitFighterState::handleAttackInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* HitFighterState::onHit(Fighter* fighter, Attack* attack)
{
	currentHitstunFrame_ = 0;//reset hitstun frame
	hitByAttack_ = attack;
	distanceToMove_ = hitByAttack_->hitPushMag;
	movePerFrame_ = (distanceToMove_ / hitByAttack_->hitstunFrames);

	if (inAir_)
		fighter->setYSpeed(10);

	comboCount_++;
	fighter->takeDamage(attack->damage);

	return nullptr;
}

BaseFighterState* HitFighterState::handleWallCollision(Fighter* fighter, bool collidedWithLeftSide)
{
	return nullptr;
}

BaseFighterState* HitFighterState::handleFloorCollision(Fighter* fighter)
{
	gravity_ = 0;
	inAir_ = false;
	return fighter->idleFighterState_;
}
