#pragma once
#include "BaseFighterState.h"
#include "../Fighter/Hitbox.h"
#include "../InputHandler.h"

const float DEFAULT_GRAVITY = -0.5f;

class JumpingFighterState :
	public BaseFighterState
{
public:
	JumpingFighterState(std::string animationName, std::vector<std::vector<Hitbox>> hitboxData, AttackResources* attacks);
	BaseFighterState* update(Fighter* fighter) override;
	void enterState(Fighter* fighter) override;
	BaseFighterState* handleMovementInput(Fighter* fighter) override;
	BaseFighterState* handleAttackInput(Fighter* fighter) override;
	BaseFighterState* onHit(Fighter* fighter, Attack* attack) override;
	BaseFighterState* handleWallCollision(Fighter* fighter, bool collidedWithLeftSide) override;
	BaseFighterState* handleFloorCollision(Fighter* fighter) override;
private:
	const float JUMP_SPEED = 30.0f;
	float gravity_ = 0;
	const std::vector<uint8_t> airDashInputLeft = {FightingGameInput::left, FightingGameInput::left};
	const std::vector<uint8_t> airDashInputRight = {FightingGameInput::right, FightingGameInput::right};
	struct
	{
		uint32_t dashFrames = 30;
		uint32_t currentDashFrame = 0;
		bool airDashing_ = false;
	} airDashResources_;
	AttackResources* attacks_;
};

