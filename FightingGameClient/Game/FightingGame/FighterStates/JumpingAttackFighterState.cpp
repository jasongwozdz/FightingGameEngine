#include "JumpingAttackFighterState.h"

#include "JumpingAttackFighterState.h"
#include "HitFighterState.h"
#include "../Fighter.h"

JumpingAttackFighterState::JumpingAttackFighterState(AttackResources* attacks) :
	BaseFighterState("", {}),
	attacks_(attacks)
{}

BaseFighterState* JumpingAttackFighterState::update(Fighter* fighter)
{
	currentAttack_->currentFrame = fighter->entity_->getComponent<Animator>().currentFrameIndex_ + 1;//get the current frame in the attack animation
	fighter->setCurrentHitboxes(currentAttack_->hitboxesPerFrame[currentAttack_->currentFrame-1]);
	fighter->currentYSpeed_ += gravity_;
	return nullptr;
}

void JumpingAttackFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(currentAttack_->animationName_);
	fighter->currentAttack_ = currentAttack_;//set this so GameStateManager has access to this fighters currentAttack and can pass it to he opposite fighter when a collision is detected
	//reset speeds
	gravity_ = DEFAULT_GRAVITY;
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
	currentAttack_->handled_ = false;
	return fighter->idleFighterState_;
}
