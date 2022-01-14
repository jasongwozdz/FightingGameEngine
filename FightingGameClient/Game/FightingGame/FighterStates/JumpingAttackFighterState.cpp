#include "JumpingAttackFighterState.h"

#include "JumpingAttackFighterState.h"
#include "HitFighterState.h"
#include "../Fighter/Fighter.h"

JumpingAttackFighterState::JumpingAttackFighterState(AttackResources* attacks) :
	BaseFighterState("", {}),
	attacks_(attacks)
{}

BaseFighterState* JumpingAttackFighterState::update(Fighter* fighter)
{
	fighter->applyGravity_ = true;
	return nullptr;
}

void JumpingAttackFighterState::enterState(Fighter* fighter)
{
	//reset speeds
	std::cout << "Enter JumpingAttackingState" << std::endl;
}

BaseFighterState* JumpingAttackFighterState::handleMovementInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* JumpingAttackFighterState::handleAttackInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* JumpingAttackFighterState::onHit(Fighter* fighter, Attack* attack)
{
	fighter->takeDamage(attack->damage);
	HitFighterState* fighterState = static_cast<HitFighterState*>(fighter->hitFighterState_);
	fighterState->inAir_ = true;
	fighterState->hitByAttack_ = attack;
	return fighter->hitFighterState_;
}

BaseFighterState* JumpingAttackFighterState::handleFloorCollision(Fighter* fighter)
{
	//transition back to idle state after landing from a jump
	//TODO: add some sort of delay after jumping
	return fighter->idleFighterState_;
}
