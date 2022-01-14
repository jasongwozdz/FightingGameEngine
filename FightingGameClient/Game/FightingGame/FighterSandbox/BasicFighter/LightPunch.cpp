#include "LightPunch.h"

#include "../../Fighter/Fighter.h"

LightPunch::LightPunch(Entity* initatingEntity, int startup, int active, int recovery, HitEffect hitEffect, std::string animationName, std::vector<FrameInfo> frameData) : 
	AttackBase(initatingEntity, startup, active, recovery, hitEffect, animationName, frameData)
{
	applyCallbackToHurtBoxes();
}

void LightPunch::initateAttack()
{
	currentFrameIndex_ = 0;
	Fighter* fighter = Fighter::getFighterComp(entity_);
	fighter->currentAttack_ = this;
	setAnimation(entity_, animationName_);
	setColliders();
}

bool LightPunch::updateAttack()
{
	currentFrameIndex_++;
	if (currentFrameIndex_ == totalFrames_)
	{
		return true;//animation is done attack is done
	}
	setAnimationTime();
	setColliders();
	return false;
}

void LightPunch::hitCallback(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	if (!handledAttack_)
	{
		Fighter* otherFighter = Fighter::getFighterComp(otherEnt);
		otherFighter->onAttackHit(hitEffect_);
	}
}

void LightPunch::applyCallbackToHurtBoxes()
{
	for (int i = 0; i < frameData_.size(); i++)
	{
		for (int j = 0; j < frameData_[i].colliders_.size(); j++)
		{
			BoxCollider& currentCollider = frameData_[i].colliders_[j];
			if (currentCollider.layer_ == HitboxLayers::HURT_BOX)
			{
				currentCollider.callback_ = BIND_COLLIDER_CALLBACK(LightPunch::hitCallback);
			}
		}
	}
}
