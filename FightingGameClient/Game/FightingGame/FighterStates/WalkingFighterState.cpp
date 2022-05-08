#include "WalkingFighterState.h"

#include <string>

#include "WalkingFighterState.h"
#include "HitFighterState.h"
#include "AttackingFighterState.h"
#include "BlockingFighterState.h"
#include "../Fighter/Fighter.h"

WalkingFighterState::WalkingFighterState(std::string animationName, std::vector<FrameInfo> frameData, AttackResources* attacks) : 
	BaseFighterState(animationName, frameData),
	attacks_(attacks)
{}

BaseFighterState* WalkingFighterState::update(Fighter* fighter)
{
	//updateCurrentHitboxes(fighter);
	if (fighter->flipSide_)
		fighter->flipSide();
	return nullptr;
}

void WalkingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	updateCurrentHitboxes(fighter);
	std::cout << "Enter walking state" << std::endl;
}

BaseFighterState* WalkingFighterState::handleMovementInput(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	setXSpeed(fighter);
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
	MoveType move = checkAttackInputsNew(fighter, *attacks_);
	if (move)
	{
		fighter->entity_->getComponent<MoveInfoComponent>().moveInfo_ = move;
		return fighter->attackingFighterState_;
	}
	return nullptr;
}

BaseFighterState* WalkingFighterState::onHit(Fighter* fighter, OnHitType hitEffect)
{
	fighter->hitFighterState_->hitByEffect_ = hitEffect;
	return fighter->hitFighterState_;
}

void WalkingFighterState::setXSpeed(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	fighter->setXSpeed(fighter->baseSpeed_ * -currentMovementInput.x);
}
