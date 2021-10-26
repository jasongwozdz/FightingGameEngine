#include "WalkingFighterState.h"

#include <string>

#include "WalkingFighterState.h"
#include "HitFighterState.h"
#include "AttackingFighterState.h"
#include "BlockingFighterState.h"
#include "../Fighter.h"

WalkingFighterState::WalkingFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData, AttackResources* attacks) : 
	BaseFighterState(animationName, hitboxData),
	attacks_(attacks)
{}

BaseFighterState* WalkingFighterState::update(Fighter* fighter)
{
	updateCurrentHitboxes(fighter);
	if (fighter->flipSide_)
		fighter->flipSide();
	return nullptr;
}

void WalkingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	setXSpeedBasedOnSide(fighter);
	std::cout << "Enter walking state" << std::endl;
}

BaseFighterState* WalkingFighterState::handleMovementInput(Fighter* fighter)
{
	setXSpeedBasedOnSide(fighter);
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	if (currentMovementInput.y > 0)
	{
		return fighter->jumpingFighterState_;
	}
	else if (currentMovementInput.y < 0)
	{
		return fighter->crouchingFighterState_;
	}
	else if (currentMovementInput.x == 0)
	{
		return fighter->idleFighterState_;
	}
	return nullptr;
}

BaseFighterState* WalkingFighterState::handleAttackInput(Fighter* fighter)
{
	Attack* attack = checkAttackInputs(fighter, *attacks_);
	if (attack)
	{
		static_cast<AttackingFighterState*>(fighter->attackingFighterState_)->currentAttack_ = attack;
		return fighter->attackingFighterState_;
	}
	return nullptr;
}

BaseFighterState* WalkingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	if (isFighterHoldingBack(fighter))
	{
		static_cast<BlockingFighterState*>(fighter->hitFighterState_)->hitByAttack_ = attack;
		return fighter->blockedFighterState_;
	}
	else
	{
		fighter->takeDamage(attack->damage);
		static_cast<HitFighterState*>(fighter->hitFighterState_)->hitByAttack_ = attack;
		return fighter->hitFighterState_;
	}
}

void WalkingFighterState::setXSpeedBasedOnSide(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	if (fighter->side_ == left)
	{
		if (currentMovementInput.x < 0)
		{
			fighter->setXSpeed(-fighter->baseSpeed_);
		}
		else
		{
			fighter->setXSpeed(fighter->baseSpeed_);
		}
	}
	else//Right side
	{
		if (currentMovementInput.x < 0)
		{
			fighter->setXSpeed(fighter->baseSpeed_);
		}
		else
		{
			fighter->setXSpeed(-fighter->baseSpeed_);
		}
	}
}

