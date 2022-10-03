#include "Fighter.h"
#include "Particles/ParticleManager.h"

#include <chrono>

#include "ResourceManager.h"
#include "../GameStateManager.h"

//debug
#include "../Application.h"

Fighter::Fighter(Entity* entity, InputHandler& inputHandler, FighterStateData idleStateData, FighterStateData walkingStateData, FighterStateData crouchStatedata, FighterStateData jumpStateData, FighterStateData hitStateData, FighterStateData blockStateData, AttackResources attacks, BoxCollider basePushBox, FighterSide side) :
	BehaviorImplementationBase(entity),
	inputHandler_(inputHandler),
	side_(side),
	attacks_(attacks)
{
	idleFighterState_ = new IdleFighterState(idleStateData.animationName, idleStateData.frameData, &attacks_);
	walkingFighterState_ = new WalkingFighterState(walkingStateData.animationName, walkingStateData.frameData, &attacks_);
	jumpingFighterState_ = new JumpingFighterState(jumpStateData.animationName, jumpStateData.frameData, &attacks_);
	jumpingAttackFighterState_ = new JumpingAttackFighterState(&attacks_);
	attackingFighterState_ = new AttackingFighterState(&attacks_);
	hitFighterState_ = new HitFighterState(hitStateData.animationName, hitStateData.frameData);
	crouchingFighterState_ = new CrouchingFighterState(crouchStatedata.animationName, crouchStatedata.frameData, &attacks_);
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
	colldingWithFighter = false;
}

bool Fighter::onAttackHit(HitEffect hitEffect)
{
	handleStateTransition(state_->onHit(this, hitEffect));
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
	if (otherEnt->name_ == "Fighter" )
	{
		switch (thisCollider->layer_)
		{
			case ColliderLayer::PUSH_BOX:
			{
				colldingWithFighter = true;
				break;
			}
			case ColliderLayer::HIT_BOX://this fighter's attack hit the other fighter
			{

				Fighter* otherFighter = Fighter::getFighterComp(otherEnt);
				MoveInfo* currentMove = entity_->getComponent<MoveInfoComponent>().moveInfo_;
				_ASSERT(currentMove);//there should always be a current move set in this case
				if (currentMove->numHits_++ < currentMove->totalHits_)
				{
					glm::vec4 tempPos = { thisCollider->position_.x, thisCollider->position_.y, thisCollider->position_.z, 1.0f };
					glm::vec3 hitBoxPos = entity_->getComponent<Transform>().calculateTransformNoScale() * tempPos;
					tempPos = { otherCollider->position_, 1.0f };
					glm::vec3 hurtBoxPos = otherEnt->getComponent<Transform>().calculateTransformNoScale() * tempPos;

					const float VELOCITY_MAG = 5.0f;

					CreateParticleInfo particleInfo;
					particleInfo.applyGravity = true;
					particleInfo.lighting = true;
					particleInfo.degreesPerFrame = 1.0f;
					particleInfo.lifeTime = 0.5f;
					particleInfo.size = 0.2f;
					particleInfo.startingPos = hurtBoxPos;
					particleInfo.velocity = otherEnt->getComponent<Transform>().position_ - entity_->getComponent<Transform>().position_;
					particleInfo.velocity *= VELOCITY_MAG;
					ParticleManager::getSingleton().addParticle(particleInfo);
					//particleInfo.velocity = entity_->getComponent<Transform>().position_ - otherEnt->getComponent<Transform>().position_;
					//particleInfo.velocity *= VELOCITY_MAG;
					//ParticleManager::getSingleton().addParticle(particleInfo);
					//particleInfo.velocity = Transform::worldUp;
					//particleInfo.velocity *= VELOCITY_MAG;
					//ParticleManager::getSingleton().addParticle(particleInfo);
					//particleInfo.velocity = Transform::worldUp * -1.0f;
					//particleInfo.velocity *= VELOCITY_MAG;
					//ParticleManager::getSingleton().addParticle(particleInfo);

					currentMove->hit_ = true;
					otherFighter->onAttackHit(currentMove->hitEffect_);
				}
				break;
			}
		}
	}
}

void Fighter::whileColliding(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	if (otherEnt->name_ == "Fighter" )
	{
		colldingWithFighter = true;
	}
}

void Fighter::onExitCollision(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	//if (otherEnt->name_ == "Fighter" /*&& thisCollider->layer_ == HitboxLayers::PUSH_BOX*/)
	//{
	//	std::cout << "exit collision" << std::endl;
	//	colldingWithFighter = false;
	//}
}

bool Fighter::isBlocking()
{
	if (side_ == left)
	{
		return inputHandler_.currentMovementInput_.x == -1;
	}
	else
	{
		return inputHandler_.currentMovementInput_.x == 1;
	}
}

