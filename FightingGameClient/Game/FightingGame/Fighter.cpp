#include <chrono>
#include "Fighter.h"
#include "ResourceManager.h"
#include "FighterStates/IdleFighterState.h"
#include "FighterStates/AttackingFighterState.h"
#include "FighterStates/WalkingFighterState.h"
#include "FighterStates/HitFighterState.h"
#include "FighterStates/JumpingFighterState.h"
#include "FighterStates/CrouchingFighterState.h"
#include "FighterStates/BlockingFighterState.h"
#include "FighterStates/JumpingAttackFighterState.h"

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterStateData crouchStatedata, FighterStateData jumpStateData, FighterStateData hitStateData, FighterStateData blockStateData, AttackResources attacks, FighterSide side) : entity_(entity), inputHandler_(inputHandler),side_(side), idleStateData_(idleStateData), walkingStateData_(walkingStateData), standingAttacks_(attacks)
{
	//enterState(idle);
	//pastTime_ = getCurrentTime();
	idleFighterState_ = new IdleFighterState(idleStateData.animationName, idleStateData.hitboxData, &standingAttacks_);
	walkingFighterState_ = new WalkingFighterState(walkingStateData.animationName, walkingStateData.hitboxData, &standingAttacks_);
	jumpingFighterState_ = new JumpingFighterState(jumpStateData.animationName, jumpStateData.hitboxData, &standingAttacks_);
	jumpingAttackFighterState_ = new JumpingAttackFighterState(&standingAttacks_);
	attackingFighterState_ = new AttackingFighterState(&standingAttacks_);
	hitFighterState_ = new HitFighterState(hitStateData.animationName, hitStateData.hitboxData);
	crouchingFighterState_ = new CrouchingFighterState(crouchStatedata.animationName, crouchStatedata.hitboxData, &standingAttacks_);
	blockedFighterState_ = new BlockingFighterState(blockStateData.animationName, blockStateData.hitboxData);
	newState_ = idleFighterState_;
	newState_->enterState(this);
}

Fighter::~Fighter()
{
	delete idleFighterState_;
	delete walkingFighterState_;
	delete jumpingFighterState_;
	delete jumpingAttackFighterState_;
	delete attackingFighterState_;
	delete crouchingFighterState_;
	delete hitFighterState_;
	delete blockedFighterState_;
}


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

void Fighter::setPosition(glm::vec3 pos)
{
entity_->getComponent<Transform>().pos_ = pos;
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
	for (Hitbox& hitbox : currentPushBoxes_)
	{
		hitbox.pos_.y *= -1;
	}

	flipSide_ = false;
}

void Fighter::updateTransform()
{
	Transform& transform = entity_->getComponent<Transform>();
	transform.pos_.y += currentXSpeed_ * deltaTime_;
	transform.pos_.z += currentYSpeed_ * deltaTime_;
}

void Fighter::handleStateTransition(BaseFighterState* transitionToState)
{
	if (transitionToState)
	{
		newState_ = transitionToState;
		newState_->enterState(this);
	}
}

void Fighter::onUpdate(float delta)
{
	deltaTime_ = delta * .001;//convert to seconds
	handleStateTransition(newState_->handleMovementInput(this));
	handleStateTransition(newState_->handleAttackInput(this));
	handleStateTransition(newState_->update(this));

	inputHandler_.updateInputQueue(deltaTime_);
}

bool Fighter::onHit(Attack& attack)
{
	handleStateTransition(newState_->onHit(this, &attack));
	return false;
}

void Fighter::setXSpeed(float speedX)
{
	currentXSpeed_ = speedX;
	if (side_ == left)
	{
		currentXSpeed_ *= -1;
	}
}

void Fighter::setYSpeed(float speedY)
{
	currentYSpeed_ = speedY;
}

void Fighter::handleWallCollision(bool collidedWithLeftWall)
{
	handleStateTransition(newState_->handleWallCollision(this, collidedWithLeftWall));
}

void Fighter::handleFloorCollision()
{
	handleStateTransition(newState_->handleFloorCollision(this));
}

void Fighter::displaceFighter(float y, float z)
{
	Transform& transform = entity_->getComponent<Transform>();
	transform.pos_.y += y;
	transform.pos_.z += z;
}

float Fighter::getXSpeed()
{
	float speedX = currentXSpeed_;
	if (side_ == left)
		speedX *= -1;
	return speedX;
}

void Fighter::takeDamage(int damage)
{
	//made this a function since this is where individual fighter damage percentages should be calculated
	health_ -= damage;
}
