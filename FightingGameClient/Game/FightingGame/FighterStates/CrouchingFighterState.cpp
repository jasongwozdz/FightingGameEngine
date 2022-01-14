#include "CrouchingFighterState.h"

#include "AttackingFighterState.h"
#include "BlockingFighterState.h"
#include "HitFighterState.h"
#include "../Fighter/Fighter.h"

CrouchingFighterState::CrouchingFighterState(std::string animationName, std::vector<FrameInfo> frameData, AttackResources* attacks) : 
	BaseFighterState(animationName, frameData),
	attacks_(attacks)
{}

BaseFighterState* CrouchingFighterState::update(Fighter* fighter)
{
	//updateCurrentHitboxes(fighter);
	if (fighter->flipSide_)
		fighter->flipSide();
	return nullptr;
}

void CrouchingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(animationName_);
	fighter->setXSpeed(0);
	std::cout << "Enter crouching state" << std::endl;
}

BaseFighterState* CrouchingFighterState::handleMovementInput(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	if (currentMovementInput.y >= 0)
	{
		return fighter->idleFighterState_;
	}
	return nullptr;
}

//BaseFighterState* CrouchingFighterState::handleAttackInput(Fighter* fighter)
//{
//	//Attack* attack = checkAttackInputs(fighter, *attacks_);
//	//if (attack)
//	//{
//	//	static_cast<AttackingFighterState*>(fighter->attackingFighterState_)->currentAttack_ = attack;
//	//	return fighter->attackingFighterState_;
//	//}
//	return nullptr;
//}

BaseFighterState * CrouchingFighterState::handleAttackInput(Fighter * fighter)
{
	AttackBase* attack = checkAttackInputsNew(fighter, *attacks_);
	if (attack)
	{
		fighter->currentAttack_ = attack;
		return fighter->jumpingAttackFighterState_;
	}
	return nullptr;
}


BaseFighterState* CrouchingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	if (isFighterHoldingBack(fighter))
	{
		static_cast<BlockingFighterState*>(fighter->blockedFighterState_)->hitByAttack_ = attack;
		return fighter->blockedFighterState_;
	}
	else
	{
		fighter->takeDamage(attack->damage);
		static_cast<HitFighterState*>(fighter->hitFighterState_)->hitByAttack_ = attack;
		return fighter->hitFighterState_;
	}
}
