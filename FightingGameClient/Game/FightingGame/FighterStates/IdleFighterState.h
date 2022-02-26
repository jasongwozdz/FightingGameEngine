#pragma once
#include "BaseFighterState.h"
#include "../Fighter/Hitbox.h"

class IdleFighterState : public BaseFighterState
{
public:
	IdleFighterState(std::string animationName, std::vector<FrameInfo> frameData, AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, OnHitType attack) override;
private:
	AttackResources* attacks_;
};
