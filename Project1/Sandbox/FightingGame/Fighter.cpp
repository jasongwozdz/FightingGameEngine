#include "Fighter.h"
#include "ResourceManager.h"
#include <chrono>

//return in milliseconds
double getCurrentTime()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	auto nano = ms.time_since_epoch();
	return nano.count();
}

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterSide side) :
	entity_(entity), inputHandler_(inputHandler),side_(side)
{
	//std::vector<uint8_t> attackInput = { Input::InputMap::down, Input::InputMap::down | Input::InputMap::right, Input::InputMap::right, Input::InputMap::light };
	std::vector<uint8_t> attackInput = { Input::InputMap::light };
	attackInputs_.push_back({ attackInput, (int)attackInput.size(), 200l});
	numAttacks_++;

	attackInput = { Input::InputMap::strong };
	attackInputs_.push_back({ attackInput, 1, 200l, 1});
	numAttacks_++;

	//attackInput = { Input::AttackMap::ultra };
	//movementInput = { {0, 0} };
	//attackInputs_.push_back({ 1, attackInput, movementInput, 3, 0});
}

void Fighter::setPosition(glm::vec3 pos)
{
	entity_->getComponent<Transform>().pos_ = pos;
}

void Fighter::flipSide()
{
	entity_->getComponent<Transform>().scale_.x *= -1;
	entity_->getComponent<Transform>().scale_.y *= -1;
	entity_->getComponent<Transform>().scale_.z *= -1;

	glm::quat rot = entity_->getComponent<Transform>().rot_;
	glm::quat flipRot(0.0f, 0.0f, 1.0f, 0.0f);
	entity_->getComponent<Transform>().rot_ = flipRot * rot;
	uint16_t swapped  = ~(uint16_t)side_;
	swapped = swapped << 15;
	swapped = swapped >> 15;
	side_ = (FighterSide)swapped;
}

void Fighter::updateTransform()
{
	glm::vec3 trans = { 0, currentMovement_.x * speed_, 0};

	Transform& transform = entity_->getComponent<Transform>();

	glm::vec3 currentPos = transform.pos_;

	currentPos += trans;

	transform.pos_ = currentPos;
}

void Fighter::handleState()
{
	switch (state_)
	{
	case FighterState::idle:
		handleMove();
		break;
	case FighterState::walking:
		handleMove();
		updateTransform();
		break;
	case FighterState::attacking:
		handleMove();
		break;
	case FighterState::hitstun:
		std::cout << "in hitstun: handle hit, stunFrames: " << stunFrames_ << "pushMag: " << pushMagnitude_ << std::endl;
		stunFrames_--;
		if (stunFrames_ != 0)
		{
			glm::vec3 pos = entity_->getComponent<Transform>().pos_;
			float pushMag = (bool)side_ ? pushMagnitude_ : pushMagnitude_ * -1 ;
			pos.y += pushMag;
			entity_->getComponent<Transform>().pos_ = pos;
		}
		else
		{
			stunFrames_ = -1;
			setOrKeepState(FighterState::idle);
		}
		break;
	}
}

void Fighter::handleMove()
{
	if (controllable_)
	{
		if (currentAttack_ != -1)
		{
			setOrKeepState(FighterState::attacking);
		}
		else if (currentMovement_.y == 0 && currentMovement_.x != 0)
		{
			setOrKeepState(FighterState::walking);
		}
		else
		{
			setOrKeepState(FighterState::idle);
		}
	}
}

void Fighter::setOrKeepState(FighterState state)
{
	if (state_ == state) return;
	else
	{
		state_ = state;
		enterState(state_);
	}
}

void Fighter::enterState(FighterState state)
{
	switch (state)
	{
	case FighterState::idle:
		entity_->getComponent<Animator>().setAnimation(-1);
		break;
	case FighterState::walking:
		entity_->getComponent<Animator>().setAnimation(0);
		break;
	case FighterState::attacking:
		entity_->getComponent<Animator>().setAnimation(-1);
		break;
	case FighterState::hitstun:
		entity_->getComponent<Animator>().setAnimation(0);
		break;
	}
}

//How to deal with input sequences
//move elements in attackInputs to have most relevant element at the beggining
//for example start with only movement.y = -1;
//move all AttackInputs that start with only movement.y to the front of the vector and increment their current input.
bool Fighter::checkAttackInput(int currentAttackInput, int& attackIndex)
{
	//std::cout << "current attack input " << currentAttackInput << std::endl;
	if (currentAttackInput == 0) return false;
	for (std::vector<AttackInput>::iterator attack = attackInputs_.begin(); attack != attackInputs_.end(); attack++)
	{

		uint8_t result = attack->attackInput[attack->currentInput] & currentAttackInput;

		if (result == attack->attackInput[attack->currentInput])
		{
			double currentTime = getCurrentTime();

			if (attack->lastCheckTime != 0)
			{
				double dt = currentTime - attack->lastCheckTime;
				std::cout << "dt " << dt << std::endl;
				if (dt > attack->dtBetweenAttacks)
				{
					//std::cout << "Execution not fast enough" << std::endl;
					attack->lastCheckTime = 0;
					attack->currentInput = 0;
					continue;
				}
				else if (dt < attackInputDelay_)
				{
					inputHandler_.currentAttackInput_ &= ~result;
					std::cout << "too fast" << std::endl;
					continue;
				}
			}
			attack->currentInput++;
			attack->lastCheckTime = currentTime;
			if (attack->numInputs == attack->currentInput)
			{
				attackIndex = attack->attackIndex;
				attack->currentInput = 0;
				attack->lastCheckTime = 0;
				return true;
			}
		}
		else
		{
			if (attack->currentInput > 0)
			{
				uint8_t lastAttack = attack->attackInput[attack->currentInput - 1];
				result = lastAttack & currentAttackInput;
				if (result == lastAttack)
				{
					continue;
				}
			}
			attack->lastCheckTime = 0;
			attack->currentInput = 0;
		}
	}
	return false;
}

void Fighter::processInput()
{
	currentMovement_ = inputHandler_.currentMovementInput_;
	int attackIndex;
	if (checkAttackInput(inputHandler_.currentAttackInput_, attackIndex))
	{
#define QUEUE_DEPTH 1
		if (attackBuffer_.size() < QUEUE_DEPTH)
		{
			attackBuffer_.push(attackIndex);
			clearAttackBufferTime_ = getCurrentTime() + timeToClearBuffer_;
		}
		std::cout << "added " << attackIndex << " to buffer" << std::endl;
	}
	inputHandler_.currentAttackInput_ = 0;
}

void Fighter::onUpdate(float delta)
{
	deltaTime_ = delta;
	if (controllable_)
	{
		processInput();
	}
	handleState();
	speed_ = baseSpeed_;
	if(attackBuffer_.size() > 0 && getCurrentTime() > clearAttackBufferTime_)
	{
		//std::cout << "buffer cleared" << std::endl;
		attackBuffer_.empty();
	}
}

bool Fighter::onHit(float pushMagnitude, int hitstunFrames, int blockStunFrames)
{
	switch (state_)
	{
		case FighterState::blocking:
		{
			stunFrames_ = hitstunFrames;
			return false;
		}
		break;
		default:
		{
			std::cout << "handle hit, stunFrames: " << stunFrames_ << "pushMag: " << pushMagnitude_ << std::endl;
			stunFrames_ = blockStunFrames;
			pushMagnitude_ = pushMagnitude;
			setOrKeepState(FighterState::hitstun);
			return true;
		}
		break;
	}
}
