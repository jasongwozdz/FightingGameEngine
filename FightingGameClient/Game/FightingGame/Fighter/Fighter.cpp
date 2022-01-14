#include "Fighter.h"

#include <chrono>

#include "ResourceManager.h"
#include "../GameStateManager.h"

//debug
#include "../Application.h"

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterStateData crouchStatedata, FighterStateData jumpStateData, FighterStateData hitStateData, FighterStateData blockStateData, AttackResources attacks, BoxCollider basePushBox, FighterSide side) :
	inputHandler_(inputHandler),
	side_(side),
	standingAttacks_(attacks),
	BehaviorImplementationBase(entity),
	basePushBox_(basePushBox)
{
	idleFighterState_ = new IdleFighterState(idleStateData.animationName, idleStateData.frameData, &standingAttacks_);
	walkingFighterState_ = new WalkingFighterState(walkingStateData.animationName, walkingStateData.frameData, &standingAttacks_);
	jumpingFighterState_ = new JumpingFighterState(jumpStateData.animationName, jumpStateData.frameData, &standingAttacks_);
	jumpingAttackFighterState_ = new JumpingAttackFighterState(&standingAttacks_);
	attackingFighterState_ = new AttackingFighterState(&standingAttacks_);
	hitFighterState_ = new HitFighterState(hitStateData.animationName, hitStateData.frameData);
	crouchingFighterState_ = new CrouchingFighterState(crouchStatedata.animationName, crouchStatedata.frameData, &standingAttacks_);
	blockedFighterState_ = new BlockingFighterState(blockStateData.animationName, blockStateData.frameData);
	state_ = idleFighterState_;
	state_->enterState(this);
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

Fighter* Fighter::getFighterComp(Entity * entity)
{
	return (Fighter*)entity->getComponent<Behavior>().behaviorImp_.get();
}

glm::vec3 Fighter::getPosition() const
{
	Transform& transform = entity_->getComponent<Transform>();
	glm::vec3 pos = transform.position_;
	return pos;
}

void Fighter::setPosition(glm::vec3 pos)
{
	entity_->getComponent<Transform>().position_ = pos;
}

void Fighter::flipSide()
{
	glm::quat rot = entity_->getComponent<Transform>().rotation_;
	glm::quat flipRot(0.0f, 0.0f, 1.0f, 0.0f);
	entity_->getComponent<Transform>().rotation_ = flipRot * rot;
	side_ = FighterSide(side_ ^ 1);
	flipSide_ = false;
}

void Fighter::updateTransform()
{
	Transform& transform = entity_->getComponent<Transform>();
	oldPos_ = transform.position_;
	if (applyGravity_)
	{
		velocityWorldSpace_ += gravity_;
	}
	transform.position_ += velocityWorldSpace_ * deltaTime_;
}

void Fighter::handleStateTransition(BaseFighterState* transitionToState)
{
	if (transitionToState)
	{
		lastState_ = state_;
		state_ = transitionToState;
		state_->enterState(this);
	}
}

void Fighter::update()
{
	deltaTime_ = Scene::DeltaTime * .001;//convert to seconds
	handleStateTransition(state_->handleMovementInput(this));
	handleStateTransition(state_->handleAttackInput(this));
	handleStateTransition(state_->update(this));
	inputHandler_.updateInputQueue(deltaTime_);
}

bool Fighter::onAttackHit(HitEffect hitEffect)
{

	return false;
}

void Fighter::setXSpeed(float speedX)
{
	velocityWorldSpace_.x = speedX;
}

void Fighter::setYSpeed(float speedY)
{
	velocityWorldSpace_.y = speedY;
}

void Fighter::handleWallCollision(bool collidedWithLeftWall)
{
	float pushOffset = 0.00f;
	if (collidedWithLeftWall)
	{
		pushOffset *= -1;
	}
	getTransform().position_.x = oldPos_.x + pushOffset;
	handleStateTransition(state_->handleWallCollision(this, collidedWithLeftWall));
}

void Fighter::handleFloorCollision()
{
	handleStateTransition(state_->handleFloorCollision(this));
}

void Fighter::takeDamage(int damage)
{
	//made this a function since this is where individual fighter damage percentages should be calculated
	health_ -= damage;
}

void Fighter::onCollision(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	if (otherEnt->name_ == "Fighter" && thisCollider->layer_ == HitboxLayers::PUSH_BOX)
	{
		colldingWithFighter = true;
	}
}

void Fighter::whileColliding(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	if (otherEnt->name_ == "Fighter" )
	{
	}
}

void Fighter::onExitCollision(Entity * otherEnt, BoxCollider * thisCollider, BoxCollider * otherCollider)
{
	if (otherEnt->name_ == "Fighter" && thisCollider->layer_ == HitboxLayers::PUSH_BOX)
	{
		colldingWithFighter = false;
	}
}
