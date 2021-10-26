#include <string>
#include "JumpingFighterState.h"
#include "BlockingFighterState.h"
#include "HitFighterState.h"
#include "JumpingAttackFighterState.h"
#include "../Fighter.h"

JumpingFighterState::JumpingFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData, AttackResources* attacks) : 
	BaseFighterState(animationName, hitboxData),
	attacks_(attacks)
{}

BaseFighterState* JumpingFighterState::update(Fighter* fighter)
{
	updateCurrentHitboxes(fighter);
	if (airDashResources_.airDashing_)//during the start of the airdash ignore gravity
	{
		if (++airDashResources_.currentDashFrame > airDashResources_.dashFrames)
		{
			gravity_ = DEFAULT_GRAVITY;
		}
	}
	fighter->currentYSpeed_ += gravity_;
	return nullptr;
}

void JumpingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	const InputHandler& inputHandler = fighter->inputHandler_;
	if (inputHandler.currentMovementInput_.x > 0)
	{
		if (fighter->side_ == left)
		{
			fighter->setXSpeed(fighter->baseSpeed_);
		}
		else
		{
			fighter->setXSpeed(-fighter->baseSpeed_);
		}
	}
	else if(inputHandler.currentMovementInput_.x < 0)
	{
		if (fighter->side_ == left)
		{
			fighter->setXSpeed(-fighter->baseSpeed_);
		}
		else
		{
			fighter->setXSpeed(fighter->baseSpeed_);
		}
	}

	fighter->setYSpeed(JUMP_SPEED);//set Y speed
	gravity_ = DEFAULT_GRAVITY;//set gravity to default value so we decrement at every state update;
	airDashResources_ = {};
	//fighter->inputHandler_.clearInputQueue();//clear input queue so no inputs while on the ground affect input checking in this state
	std::cout << "Enter jumping state" << std::endl;
}

BaseFighterState* JumpingFighterState::handleMovementInput(Fighter* fighter)
{
	//TODO: implement air dashing
	const InputHandler& inputHandler = fighter->inputHandler_;
	if (!airDashResources_.airDashing_)
	{
		const float AIR_DASH_SPEED = -20.0f;
		if (inputHandler.isSequenceInInputQueue(airDashInputLeft))
		{
			if (fighter->side_ == left)
				fighter->setXSpeed(AIR_DASH_SPEED);
			else
				fighter->setXSpeed(-AIR_DASH_SPEED);

			fighter->setYSpeed(0);
			airDashResources_.airDashing_ = true;
			gravity_ = 0;
		}
		else if (inputHandler.isSequenceInInputQueue(airDashInputRight))
		{
			if (fighter->side_ == left)
				fighter->setXSpeed(-AIR_DASH_SPEED);
			else
				fighter->setXSpeed(AIR_DASH_SPEED);

			fighter->setYSpeed(0);
			airDashResources_.airDashing_ = true;
			gravity_ = 0;
		}
	}
	return nullptr;
}

BaseFighterState* JumpingFighterState::handleAttackInput(Fighter* fighter)
{
	Attack* attack = checkAttackInputs(fighter, *attacks_);
	if (attack)
	{
		static_cast<JumpingAttackFighterState*>(fighter->jumpingAttackFighterState_)->currentAttack_ = attack;
		return fighter->jumpingAttackFighterState_;
	}
	return nullptr;
}

BaseFighterState* JumpingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	
	if (isFighterHoldingBack(fighter))
	{
		BlockingFighterState* fighterState = static_cast<BlockingFighterState*>(fighter->hitFighterState_);
		fighterState->inAir_ = true;
		fighterState->hitByAttack_ = attack;
		return fighter->blockedFighterState_;
	}
	else
	{
		fighter->takeDamage(attack->damage);
		HitFighterState* fighterState = static_cast<HitFighterState*>(fighter->hitFighterState_);
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
