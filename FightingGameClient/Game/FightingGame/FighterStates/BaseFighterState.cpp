#include "BaseFighterState.h"

#include "../Fighter.h"

void BaseFighterState::updateCurrentHitboxes(Fighter* fighter)
{
	const Animator& animator = fighter->entity_->getComponent<Animator>();
	uint32_t frameIndex = animator.currentFrameIndex_;
	if (frameIndex > hitboxData_.size())
	{
		printf("ERROR: no hitbox data for frameIndex: %i in State Idle", frameIndex);
	}
	else
	{
		fighter->setCurrentHitboxes(hitboxData_[frameIndex]);
	}
}

Attack* BaseFighterState::checkAttackInputs(Fighter* fighter, AttackResources& attacks)
{
	Attack* currentAttack = nullptr;
	InputHandler& inputHandler = fighter->inputHandler_;
	std::queue<std::deque<Input::InputTime>::iterator> toBeDeleted;
	int attackIndex = 0;
	for (AttackInput attack : attacks.inputs_)//loop through each attack in AttackResources
	{
		uint32_t currentAttackInputIndex = 0;
		for (auto input = inputHandler.inputQueue_.begin(); input < inputHandler.inputQueue_.end(); input++)
		{
			uint8_t currentAttackInput = attack.attackInput[currentAttackInputIndex];
			if (input->first & currentAttackInput)
			{
				if (input->first >= Input::Action::light)//remove any inputs in queue that are attacks if we use them
				{
					toBeDeleted.push(input);
				}

				if (++currentAttackInputIndex >= attack.attackInput.size())//Succesful attack input
				{
					while (!toBeDeleted.empty())//delete all attack inputs that were using for this input
					{
						inputHandler.inputQueue_.erase(toBeDeleted.front());
						toBeDeleted.pop();
					}
					currentAttack = &attacks.attacks_[attackIndex];
					return currentAttack;
				}
			}
		}
		attackIndex++;
	}

	return currentAttack;
}

bool BaseFighterState::isFighterHoldingBack(Fighter* fighter)
{
	const InputHandler& inputHandler = fighter->inputHandler_;
	if (fighter->side_ == left)
	{
		if (inputHandler.currentMovementInput_.x > 0)
			return true;
	}
	else
	{
		if (inputHandler.currentMovementInput_.x < 0)
			return true;
	}
	return false;
}
