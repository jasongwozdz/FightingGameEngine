#include "Fighter.h"
#include "ResourceManager.h"

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterSide side) :
	entity_(entity), inputHandler_(inputHandler),side_(side)
{
	std::vector<uint8_t> attackInput = { Input::InputMap::light };
	std::vector<glm::vec2> movementInput = { {0, 0} };
	attackInputs_.push_back({ 1, attackInput, movementInput, 0 , 0});
	numAttacks_++;

	attackInput = { Input::InputMap::nothing, Input::InputMap::nothing, Input::InputMap::medium };
	movementInput = { {-1, 0} , {-1, 1}, {1, 0} };
	attackInputs_.push_back({ 1, attackInput, movementInput, 1 , 0});
	numAttacks_++;

	//attackInput = { Input::AttackMap::strong };
	//movementInput = { {0, 0} };
	//attackInputs_.push_back({ 1, attackInput, movementInput, 2, 0});

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
		if (stunFrames_ != -1)
		{
			glm::vec3 pos = entity_->getComponent<Transform>().pos_;
			float pushMag = (bool)side_ ? pushMagnitude_ : pushMagnitude_ * -1 ;
			pos.y += pushMag;
			entity_->getComponent<Transform>().pos_ = pos;
		}
		else
		{
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
	for (std::vector<AttackInput>::iterator iter = attackInputs_.begin(); iter != attackInputs_.end(); iter++)
	{
		int result = iter->attackInput[iter->currentInput] & currentAttackInput;
		if (result > 0)
		{
			iter->currentInput++;
			if (iter->numInputs == iter->currentInput)
			{
				attackIndex = iter->attackIndex;
				iter->currentInput = 0;
				return true;
			}
		}
		else
		{
			iter->currentInput = 0;
		}
	}
	return false;
}

//std::vector<int> Fighter::checkMovementInput(glm::vec2 currMovement)
//{
//	for (std::vector<AttackInput>::iterator attack = attackInputs_.begin(); attack != attackInputs_.end(); attack++)
//	{
//		float currentTime = std::clock();
//		if (attack->lastCheckTime != 0)
//		{
//			float dt = currentTime - attack->lastCheckTime;
//			if (dt > attack->dtBetweenAttacks)
//			{
//				attack->lastCheckTime = 0;
//				attack->attackIndex = 0;
//				attack->movementIndex = 0;
//				continue;
//			}
//		}
//	}
//}

void Fighter::processInput()
{
	currentMovement_ = inputHandler_.currentMovementInput_;
	int attackIndex;
	if (checkAttackInput(inputHandler_.currentAttackInput_, attackIndex))
	{
		currentAttack_ = attackIndex;
	}
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
