#pragma once
#include "BaseFighterState.h"
#include "../Fighter/Hitbox.h"
#include "../FighterSandbox/AttackTypes.h"

struct Attack;
struct AttackResources;
class MoveHandler;

class AttackingFighterState :
	public BaseFighterState
{
public:
	AttackingFighterState(AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, OnHitType attack) override;
	BaseFighterState* handleFloorCollision(Fighter* fighter) override;
public:
	MoveType currentMove_;

private:
	AttackResources* attacks_;//need to store attacks to check if the current attack can be cancelled
	FrameType attackPhase_ = STARTUP;
	MoveHandler* moveHandler_;
};

