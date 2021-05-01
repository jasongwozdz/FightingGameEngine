#include "Fighter.h"
#include "ResourceManager.h"

Fighter::Fighter(Entity* entity, InputHandler& inputHandler) :
	entity_(entity), inputHandler_(inputHandler)
{
	std::vector<int> attackInput = { Input::AttackMap::light };
	std::vector<glm::vec2> movementInput = { {0, 0} };
	attackInputs_.push_back({ 1, attackInput, movementInput, 0 , 0});

	//attackInput = { Input::AttackMap::medium };
	//movementInput = { {0, 0} };
	//attackInputs_.push_back({ 1, attackInput, movementInput, 1 , 0});

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
	}
}


//How to deal with input sequences
//move elements in attackInputs to have most relevant element at the beggining
//for example start with only movement.y = -1;
//move all AttackInputs that start with only movement.y to the front of the vector and increment their current input.
bool Fighter::checkAttackInput(glm::vec2& currentMovement, int currentAttackInput, int& attackIndex)
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

void Fighter::processInput()
{
	currentMovement_ = inputHandler_.currentInput_;
	int attackIndex;
	if (checkAttackInput(currentMovement_, inputHandler_.currentAttackInput_, attackIndex))
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
