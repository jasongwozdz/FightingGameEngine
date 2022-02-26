#include "HitHandler.h"
#include "../Fighter/Fighter.h"

HitHandler::HitHandler()
{}

HitHandler::~HitHandler()
{}

bool HitHandler::updateHit(Fighter* fighter)
{
	if (currentFrame_ >= hitEffect_.freezeFrames_ + stunFrames_)
	{
		return true;//hit is finished
	}
	
	Animator& animator = fighter->entity_->getComponent<Animator>();
	AnimationClip* clip = animator.getAnimationClipByName("Hit");
	float ratio = currentFrame_ / (float)totalFrames_;
	float timeInSeconds = clip->durationInSeconds_ * ratio;
	animator.setAnimationTime(timeInSeconds);

	if (currentFrame_ > hitEffect_.freezeFrames_)//currently in stun frames
	{
		fighter->velocityWorldSpace_ = pushVelocity_;
	}

	currentFrame_++;
	return false;
}

void HitHandler::setHitEffect(Fighter* fighter, HitEffect hitEffect, bool attackHit)
{
	Animator& animator = fighter->entity_->getComponent<Animator>();
	animator.setAnimation("Hit");
	hitEffect_ = hitEffect;
	stunFrames_ = attackHit ? hitEffect_.hitstun_ : hitEffect_.blockstun_;
	totalFrames_ = stunFrames_ + hitEffect.freezeFrames_;
	currentFrame_ = 0;

	Transform& transform = fighter->entity_->getComponent<Transform>();
	glm::vec3 push = transform.forward() * hitEffect_.pushBack_ * -1.0f;
	push.z = 0;
	finalPos_ = transform.position_ + push;
	
	float distance = glm::length(finalPos_ - transform.position_);
	float pushMag = distance / totalFrames_;
	pushVelocity_ = transform.forward() * pushMag * -1.0f;
}
