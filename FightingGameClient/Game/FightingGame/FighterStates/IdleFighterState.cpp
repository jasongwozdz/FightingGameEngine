#include <string>

#include "IdleFighterState.h"
#include "AttackingFighterState.h"
#include "BlockingFighterState.h"
#include "HitFighterState.h"
#include "../Fighter.h"


IdleFighterState::IdleFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData, AttackResources* attacks) :
	BaseFighterState(animationName, hitboxData),
	attacks_(attacks)
{}

BaseFighterState* IdleFighterState::update(Fighter* fighter)
{
	updateCurrentHitboxes(fighter);
	if (fighter->flipSide_)
		fighter->flipSide();
	return nullptr;
}

void IdleFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	//reset speeds
	fighter->setXSpeed(0);
	fighter->setYSpeed(0);
	std::cout << "Enter IdleState" << std::endl;
}

BaseFighterState* IdleFighterState::handleMovementInput(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	if (currentMovementInput.x != 0 && currentMovementInput.y == 0)
	{
		return fighter->walkingFighterState_;
	}
	else if (currentMovementInput.y > 0)
	{
		return fighter->jumpingFighterState_;
	}
	else if (currentMovementInput.y < 0)
	{
		return fighter->crouchingFighterState_;
	}
	return nullptr;
}

BaseFighterState* IdleFighterState::handleAttackInput(Fighter* fighter)
{
	Attack* attack = checkAttackInputs(fighter, *attacks_);
	if (attack)
	{
		static_cast<AttackingFighterState*>(fighter->attackingFighterState_)->currentAttack_ = attack;
		return fighter->attackingFighterState_;
	}
	return nullptr;
}

BaseFighterState* IdleFighterState::onHit(Fighter* fighter, Attack* attack)
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
