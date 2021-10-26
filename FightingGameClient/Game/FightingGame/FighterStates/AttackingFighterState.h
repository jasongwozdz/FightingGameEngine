#pragma once
#include "BaseFighterState.h"
#include "../Hitbox.h"

struct Attack;
struct AttackResources;

enum AttackPhase
{
	STARTUP,
	ACTIVE,
	RECOVERY
};

class AttackingFighterState :
	public BaseFighterState
{
public:
	AttackingFighterState(AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, Attack* attack) override;
public:
	Attack* currentAttack_;
private:
	AttackResources* attacks_;//need to store attacks to check if the current attack can be cancelled
	AttackPhase attackPhase_ = STARTUP;
};

