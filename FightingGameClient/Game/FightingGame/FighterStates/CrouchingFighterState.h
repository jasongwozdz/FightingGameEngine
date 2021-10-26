#pragma once
#include "BaseFighterState.h"

class CrouchingFighterState :
	public BaseFighterState
{
public:
	CrouchingFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData, AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, Attack* attack) override;
private:
	AttackResources* attacks_;
};

