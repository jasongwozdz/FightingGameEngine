#include "AttackBase.h"

#include "AttackTypes.h"
#include "Scene/Scene.h"
#include "Scene/Components/Collider.h"
#include "Scene/Components/Animator.h"
#include "../Fighter/Fighter.h"

AttackBase::AttackBase(Entity* initatingEntity, int startup, int active, int recovery, HitEffect hitEffect, std::string animationName, std::vector<FrameInfo> frameData) :
	startupFrames_(startup),
	activeFrames_(active),
	recoveryFrames_(recovery),
	scene_(Scene::getSingletonPtr()),
	animationName_(animationName),
	entity_(initatingEntity),
	hitEffect_(hitEffect),
	frameData_(frameData),
	currentFrameIndex_(0),
	totalFrames_(startup + active + recovery)
{}

void AttackBase::initateAttack()
{
	currentFrameIndex_ = 0;
	Fighter* fighter = Fighter::getFighterComp(entity_);
	fighter->currentAttack_ = this;
	setAnimation(entity_, animationName_);
	setColliders();
}

bool AttackBase::updateAttack()
{
	currentFrameIndex_++;
	if (currentFrameIndex_ >= totalFrames_)
	{
		Fighter* fighter = Fighter::getFighterComp(entity_);
		fighter->currentAttack_ = nullptr;
		return true;//animation is done attack is done
	}
	setAnimationTime();
	setColliders();
	return false;
}

Entity* AttackBase::createNewEntity(std::string name)
{
	return scene_->addEntity(name);
}

void AttackBase::addCollider(Entity* attachTo, glm::vec3 pos, glm::vec3 size, int layer)
{
	BoxCollider boxCollider = BoxCollider(size, pos, layer);
	Collider* collider = attachTo->tryGetComponent<Collider>();
	if (!collider)
	{
		Collider newCollider(attachTo);
		newCollider.colliders_.push_back(boxCollider);
	}
	else
	{
		collider->colliders_.push_back(boxCollider);
	}
}

void AttackBase::addBehavior(Entity* entity, BehaviorImplementationBase* behavior)
{
	entity->addComponent<Behavior>(behavior);
}

void AttackBase::setEntityPosition(glm::vec3 position, Entity* entity)
{
	Transform& transform = entity->getComponent<Transform>();
	transform.position_ = position;
}

void AttackBase::displaceEntity(glm::vec3 displacement, Entity * entity)
{
	Transform& transform = entity->getComponent<Transform>();
	transform.position_ += displacement;
}

void AttackBase::setAnimation(Entity* entity, std::string animationName)
{
	Animator* animator = entity->tryGetComponent<Animator>();
	int animationIndex = animator->findAnimationIndexByName(animationName);
	if (animationIndex != -1)
	{
		animator->setAnimation(animationIndex);
	}
	else
	{
		std::cout << "ERROR: animation name not found" << std::endl;
	}
}

void AttackBase::setAnimationTime()
{
	Animator& animator = entity_->getComponent<Animator>();
	AnimationClip* clip = animator.getAnimationClipByName(animationName_);
	float ratio = currentFrameIndex_ / (float)totalFrames_;
	float timeInSeconds = clip->durationInSeconds_ * ratio;
	animator.setAnimationTime(timeInSeconds);
}

void AttackBase::setColliders()
{
	entity_->getComponent<Collider>().colliders_ = frameData_[currentFrameIndex_].colliders_;
}

void AttackBase::readFrameTags()
{
	FrameInfo currentFrame = frameData_[currentFrameIndex_];
}

int AttackBase::getAnimFrame()
{
	int currentFrame = entity_->getComponent<Animator>().currentFrameIndex_;
	return currentFrame;
}
