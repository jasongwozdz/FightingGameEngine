#pragma once
#include "AttackTypes.h"

class Fighter;

class HitHandler
{
public:
	HitHandler();
	~HitHandler();

	bool updateHit(Fighter* fighter);
	void setHitEffect(Fighter* fighter, HitEffect hitEffect, bool attackHit);
private:
	HitEffect hitEffect_;
	int currentFrame_;
	int stunFrames_;
	int totalFrames_;
	glm::vec3 finalPos_;
	glm::vec3 pushVelocity_;
};
