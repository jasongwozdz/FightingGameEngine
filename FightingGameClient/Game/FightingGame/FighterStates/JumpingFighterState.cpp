#include <string>
#include "JumpingFighterState.h"
#include "BlockingFighterState.h"
#include "HitFighterState.h"
#include "JumpingAttackFighterState.h"
#include "../Fighter/Fighter.h"

JumpingFighterState::JumpingFighterState(std::string animationName, std::vector<FrameInfo> frameData, AttackResources* attacks) : 
	BaseFighterState(animationName, frameData),
	attacks_(attacks)
{}

BaseFighterState* JumpingFighterState::update(Fighter* fighter)
{
	//updateCurrentHitboxes(fighter);
	if (airDashResources_.airDashing_)//during the start of the airdash ignore gravity
	{
		if (++airDashResources_.currentDashFrame > airDashResources_.dashFrames)
		{
			fighter->applyGravity_ = true;
		}
	}
	return nullptr;
}

void JumpingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	const InputHandler& inputHandler = fighter->inputHandler_;
	fighter->setYSpeed(fighter->jumpSpeed_);//set Y speed
	fighter->applyGravity_= true;
	//gravity_ = DEFAULT_GRAVITY;//set gravity to default value so we decrement at every state update;
	airDashResources_ = {};
	//fighter->inputHandler_.clearInputQueue();//clear input queue so no inputs while on the ground affect input checking in this state
	std::cout << "Enter jumping state" << std::endl;
}

BaseFighterState* JumpingFighterState::handleMovementInput(Fighter* fighter)
{
	const InputHandler& inputHandler = fighter->inputHandler_;
	if (!airDashResources_.airDashing_)
	{
		if (inputHandler.isSequenceInInputQueue(airDashInputLeft))
		{
			fighter->setXSpeed(fighter->airDashSpeed_);
			fighter->setYSpeed(0);
			airDashResources_.airDashing_ = true;
			fighter->applyGravity_ = false;
		}
		else if (inputHandler.isSequenceInInputQueue(airDashInputRight))
		{
			fighter->setXSpeed(-fighter->airDashSpeed_);
			fighter->setYSpeed(0);
			airDashResources_.airDashing_ = true;
			fighter->applyGravity_ = false;
		}
	}
	return nullptr;
}

BaseFighterState* JumpingFighterState::handleAttackInput(Fighter* fighter)
{
	AttackBase* attack = checkAttackInputsNew(fighter, *attacks_);
	if (attack)
	{
		attack->initateAttack();
		return fighter->jumpingAttackFighterState_;
	}
	return nullptr;
}

BaseFighterState* JumpingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	
	if (isFighterHoldingBack(fighter))
	{
		BlockingFighterState* fighterState = fighter->blockedFighterState_;
		fighterState->inAir_ = true;
		fighterState->hitByAttack_ = attack;
		return fighter->blockedFighterState_;
	}
	else
	{
		fighter->takeDamage(attack->damage);
		HitFighterState* fighterState = fighter->hitFighterState_;
		fighterState->inAir_ = true;
		fighterState->hitByAttack_ = attack;
		return fighter->hitFighterState_;
	}

}

BaseFighterState* JumpingFighterState::handleWallCollision(Fighter* fighter, bool collidedWithLeftSide)
{
//TODO: implement wall bounce
	return nullptr;
}

BaseFighterState* JumpingFighterState::handleFloorCollision(Fighter* fighter)
{
	//transition back to idle state after landing from a jump
	//TODO: add some sort of delay after jumping
	//gravity_ = 0;
	//fighter->currentYSpeed_ = 0;
	//fighter->setXSpeed(0);
	//enter idle state reset y and x speed
	return fighter->idleFighterState_;
}
