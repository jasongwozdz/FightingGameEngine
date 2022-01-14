#include "AttackingFighterState.h"
#include "HitFighterState.h"
#include "../Fighter/Fighter.h"

AttackingFighterState::AttackingFighterState(AttackResources* attacks) :
	BaseFighterState("", {}),
	attacks_(attacks)
{}

BaseFighterState* AttackingFighterState::update(Fighter* fighter)
{
	if (currentAttack_->updateAttack())
	{
		return fighter->lastState_;
	}
	return nullptr;
}

void AttackingFighterState::enterState(Fighter* fighter)
{
	currentAttack_ = fighter->currentAttack_;
	//reset speeds
	fighter->setXSpeed(0);
	fighter->setYSpeed(0);
	std::cout << "Enter AttackingState" << std::endl;
}

BaseFighterState* AttackingFighterState::handleMovementInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* AttackingFighterState::handleAttackInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* AttackingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	fighter->takeDamage(attack->damage);
	static_cast<HitFighterState*>(fighter->hitFighterState_)->hitByAttack_ = attack;
	return fighter->hitFighterState_;
	return fighter->hitFighterState_;
}

BaseFighterState* AttackingFighterState::handleFloorCollision(Fighter* fighter)
{
	fighter->currentAttack_ = nullptr;
	return fighter->idleFighterState_;
}
