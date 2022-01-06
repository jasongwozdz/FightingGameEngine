#pragma once
#include "BaseFighterState.h"

extern const float DEFAULT_GRAVITY;

class JumpingAttackFighterState :
	public BaseFighterState
{
public:
	JumpingAttackFighterState(AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, Attack* attack) override;
	BaseFighterState* handleFloorCollision(Fighter* fighter) override;
public:
	Attack* currentAttack_;
private:
	AttackResources* attacks_;//need to store attacks to check if the current attack can be cancelled
	glm::vec3 gravity_ = { 0.0f, -1.0f, 0.0f };
};

