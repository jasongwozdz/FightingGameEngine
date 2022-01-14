#pragma once
#include "../AttackBase.h"
#include "../AttackTypes.h"

class LightPunch : public AttackBase
{
public:
	LightPunch(Entity* initatingEntity, int startup, int active, int recovery, HitEffect hitEffect, std::string animationName, std::vector<FrameInfo> frameData);
	// Inherited via AttackBase
	virtual void initateAttack() override;
	virtual bool updateAttack() override;
private:
	void hitCallback(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	void applyCallbackToHurtBoxes();

private:
	bool handledAttack_ = false;

};
