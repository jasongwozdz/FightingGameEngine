#include "AttackingFighterState.h"
#include "HitFighterState.h"
#include "../Fighter/Fighter.h"
#include "../FighterSandbox/MoveHandler.h"

AttackingFighterState::AttackingFighterState(AttackResources* attacks) :
	BaseFighterState("", {}),
	attacks_(attacks),
	moveHandler_(new MoveHandler)
{}

BaseFighterState* AttackingFighterState::update(Fighter* fighter)
{
	if(moveHandler_->updateMove(fighter))
	{
		return fighter->lastState_;
	}
	return nullptr;
}

void AttackingFighterState::enterState(Fighter* fighter)
{
	std::cout << "Enter AttackingState" << std::endl;
	MoveInfo* moveInfo = fighter->entity_->getComponent<MoveInfoComponent>().moveInfo_;
	moveHandler_->setMove(fighter, moveInfo);
}

BaseFighterState* AttackingFighterState::handleMovementInput(Fighter* fighter)
{
	return nullptr;
}

BaseFighterState* AttackingFighterState::handleAttackInput(Fighter* fighter)
{
	MoveType nextMove = checkAttackInputsNew(fighter, fighter->attacks_);
	if (nextMove)
	{
		moveHandler_->addNextMove(nextMove);
	}
	return nullptr;
}

BaseFighterState* AttackingFighterState::onHit(Fighter* fighter, OnHitType attack)
{
	//fighter->currentAttack_ = nullptr;
	return fighter->idleFighterState_;
}

BaseFighterState* AttackingFighterState::handleFloorCollision(Fighter* fighter)
{
	//fighter->currentAttack_ = nullptr;
	return fighter->idleFighterState_;
}
