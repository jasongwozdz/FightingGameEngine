#include "AttackingFighterState.h"
#include "HitFighterState.h"
#include "../Fighter.h"

AttackingFighterState::AttackingFighterState(AttackResources* attacks) :
	BaseFighterState("", {}),
	attacks_(attacks)
{}

BaseFighterState* AttackingFighterState::update(Fighter* fighter)
{
	currentAttack_->currentFrame = fighter->entity_->getComponent<Animator>().currentFrameIndex_ + 1;//get the current frame in the attack animation
	fighter->setCurrentHitboxes(currentAttack_->hitboxesPerFrame[currentAttack_->currentFrame-1]);
	return nullptr;
}

void AttackingFighterState::enterState(Fighter* fighter)
{
	fighter->entity_->getComponent<Animator>().setAnimation(currentAttack_->animationName_);
	fighter->currentAttack_ = currentAttack_;//set this so GameStateManager has access to this fighters currentAttack and can pass it to he opposite fighter when a collision is detected
	//reset speeds
	fighter->setXSpeed(0);
	fighter->setYSpeed(0);
	std::cout << "Enter AttackingState" << std::endl;
}

BaseFighterState* AttackingFighterState::handleMovementInput(Fighter* fighter)
{
	glm::vec2 currentMovementInput = fighter->inputHandler_.currentMovementInput_;
	//once the atack is finished set the new state
	if (currentAttack_->currentFrame == currentAttack_->getTotalFrames())
	{
		currentAttack_->handled_ = false;
		if (currentMovementInput.x > 0 && currentMovementInput.y == 0)
		{
			return fighter->walkingFighterState_;
		}
		else if (currentMovementInput.y > 0)
		{
			return fighter->jumpingFighterState_;
		}
		else
		{
			return fighter->idleFighterState_;
		}
	}
	return nullptr;
}

BaseFighterState* AttackingFighterState::handleAttackInput(Fighter* fighter)
{
	//Attack* attack = checkAttackInputs(fighter, attacks_);
	//if (attack)
	//{
	//	dynamic_cast<AttackingFighterState*>(fighter->attackingFighterState_)->currentAttack_ = attack;
	//	return fighter->attackingFighterState_;
	//}
	return nullptr;
}

BaseFighterState* AttackingFighterState::onHit(Fighter* fighter, Attack* attack)
{
	fighter->takeDamage(attack->damage);
	static_cast<HitFighterState*>(fighter->hitFighterState_)->hitByAttack_ = attack;
	return fighter->hitFighterState_;
	return fighter->hitFighterState_;
}
