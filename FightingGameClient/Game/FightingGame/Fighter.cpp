#include <chrono>
#include "Fighter.h"
#include "ResourceManager.h"

//return in milliseconds
double getCurrentTime()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	auto nano = ms.time_since_epoch();
	return nano.count();
}

glm::vec3 Fighter::getPosition() const
{
	Transform& transform = entity_->getComponent<Transform>();
	glm::vec3 pos = transform.pos_;
	return pos;
}

void Fighter::setCurrentHitboxes(const std::vector<Hitbox>& hitboxes)
{
	currentHitboxes_.clear();
	currentHurtboxes_.clear();
	currentPushBoxes_.clear();
	for (Hitbox hitbox : hitboxes)
	{
		if(side_ == left)
		{ 
			hitbox.pos_.y *= -1;
		}

		switch (hitbox.layer_)
		{
			case Hitbox::HitboxLayer::Push:
			{
				
				currentPushBoxes_.push_back(hitbox);
				break;
			}
			case Hitbox::HitboxLayer::Hit:
			{
				currentHitboxes_.push_back(hitbox);
				break;
			}
			case Hitbox::HitboxLayer::Hurt:
			{
				currentHurtboxes_.push_back(hitbox);
				break;
			}
		}
	}
}

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterSide side) :
	entity_(entity), inputHandler_(inputHandler),side_(side), idleStateData_(idleStateData), walkingStateData_(walkingStateData)
{
	enterState(idle);
	pastTime_ = getCurrentTime();
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
	side_ = FighterSide(side_ ^ 1);

	for (Hitbox& hitbox : currentHitboxes_)
	{
		hitbox.pos_.y *= -1;
	}
	for (Hitbox& hitbox : currentHurtboxes_)
	{
		hitbox.pos_.y *= -1;
	}
	for (Hitbox& hitbox : currentHitboxes_)
	{
		hitbox.pos_.y *= -1;
	}

	flipSide_ = false;
}

void Fighter::updateTransform()
{
	Transform& transform = entity_->getComponent<Transform>();
	transform.pos_.y += currentXspeed_ ;
	currentYspeed_ += gravity_ * deltaTime_;
	transform.pos_.z += currentYspeed_;
	gamePos_ = transform.pos_;
}

void Fighter::handleState()
{
	switch (state_)
	{
	case FighterState::idle:
	case FighterState::walkingBackward:
	case FighterState::walkingForward:
	case FighterState::crouching:
		if (flipSide_) flipSide();
		handleMove();
		if (attackBuffer_.size() > 0)
		{
			currentAttack_ = attackBuffer_.front();
			attackBuffer_.pop();
		}
		break;
	case FighterState::jumping:
		break;
	case FighterState::attacking:
		handleMove();
		break;
	case FighterState::hitstun:
		stunFrames_--;
		if (stunFrames_ > 0)
		{
			glm::vec3 pos = entity_->getComponent<Transform>().pos_;
			float pushMag = (bool)side_ ? pushMagnitude_ : pushMagnitude_ * -1 ;
			pos.y += pushMag;
			currentXspeed_ = 0;
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
		else if (currentMovement_.y < 0)
		{
			setOrKeepState(FighterState::crouching);
		}
		else if (currentMovement_.y == 0 && currentMovement_.x > 0)
		{
			if (side_ == left)
			{
				setOrKeepState(FighterState::walkingForward);
			}
			else
			{
				setOrKeepState(FighterState::walkingBackward);
			}
		}
		else if (currentMovement_.y == 0 && currentMovement_.x < 0)
		{
			if (side_ == left)
			{
				setOrKeepState(FighterState::walkingBackward);
			}
			else
			{
				setOrKeepState(FighterState::walkingForward);
			}
		}
		else if (currentMovement_.y > 0)
		{
			setOrKeepState(FighterState::jumping);
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
		entity_->getComponent<Animator>().setAnimation(idleStateData_.animationName);
		setCurrentHitboxes(idleStateData_.hitboxData[0]);
		break;
	case FighterState::walkingForward:
		entity_->getComponent<Animator>().setAnimation(walkingStateData_.animationName);
		setCurrentHitboxes(walkingStateData_.hitboxData[0]);
		break;
	case FighterState::walkingBackward:
		entity_->getComponent<Animator>().setAnimation(walkingStateData_.animationName);
		setCurrentHitboxes(walkingStateData_.hitboxData[0]);
		break;
	case FighterState::crouching:
		entity_->getComponent<Animator>().setAnimation(-1);
		currentXspeed_ = 0.0f;
		break;
	case FighterState::attacking:
		entity_->getComponent<Animator>().setAnimation(attacks_[currentAttack_].animationName_);
		currentXspeed_ = 0;
		break;
	case FighterState::hitstun:
		entity_->getComponent<Animator>().setAnimation(-1);
		break;
	case FighterState::jumping:
		currentYspeed_ = terminalYSpeed_;
		currentXspeed_ = 0;
		if (currentMovement_.x < 0)
		{
			currentXspeed_ = -baseSpeed_* 2 * deltaTime_;
		}
		else if (currentMovement_.x > 0)
		{
			currentXspeed_ = baseSpeed_ * 2 * deltaTime_;
		}
		break;
	}
}

bool Fighter::checkAttackInput(int currentAttackInput, int& attackIndex)
{
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
	if (state_ == idle || state_ == walkingForward || state_ == walkingBackward)
	{
		currentXspeed_ = baseSpeed_ * currentMovement_.x * deltaTime_;
	}
	int attackIndex;
	if (checkAttackInput(inputHandler_.currentAttackInput_, attackIndex))
	{
#define QUEUE_DEPTH 1
		if (attackBuffer_.size() < QUEUE_DEPTH)
		{
			attackBuffer_.push(attackIndex);
			clearAttackBufferTime_ = getCurrentTime() + timeToClearBuffer_;
		}
	}
	inputHandler_.currentAttackInput_ = 0;
}

void Fighter::onUpdate(float delta)
{
	double currentTime = getCurrentTime();
	deltaTime_ = (currentTime - pastTime_)/1000;
	pastTime_ = currentTime;
	if (controllable_)
	{
		processInput();
	}
	if (side_ == left)
		std::cout << getPosition().y << std::endl;
	handleState();
	updateTransform();
	speed_ = baseSpeed_ * deltaTime_;
}

bool Fighter::onHit(float pushMagnitude, int hitstunFrames, int blockStunFrames)
{
	if (side_ == left && inputHandler_.currentMovementInput_.x == -1)
	{
		stunFrames_ = blockStunFrames;
		setOrKeepState(FighterState::hitstun);
		return false;
	}
	else if (inputHandler_.currentMovementInput_.x == 1)
	{
		stunFrames_ = blockStunFrames;
		setOrKeepState(FighterState::hitstun);
		return false;

	}
	else
	{
		stunFrames_ = hitstunFrames;
		pushMagnitude_ = pushMagnitude;
		setOrKeepState(FighterState::hitstun);
		return true;
	}
}
