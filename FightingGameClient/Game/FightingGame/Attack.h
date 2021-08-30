#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Hitbox.h"

struct Attack
{
	Attack() = default;

	Attack(int startup, int active, int recovery, std::vector<glm::vec2> hurtboxWidthHeightIn, std::vector<glm::vec2> hurtboxPosIn, int animationIndexIn, Hitbox hurtboxIn, int blockstun, int hitstun, float pushMag, int dam) :
		startupFrames(startup), activeFrames(active), recoveryFrames(recovery),hurtboxWidthHeight(hurtboxWidthHeightIn), hurtboxPos(hurtboxPosIn), animationIndex_(animationIndexIn), hurtbox_(hurtboxIn), blockstunFrames(blockstun), hitstunFrames(hitstun), hitPushMag(pushMag), damage(dam)
	{}

	int currentFrame = 0;

	int startupFrames; //no hurtox active
	int activeFrames; //hurtbox active
	int recoveryFrames; //no hurtbox active

	int blockstunFrames;
	int hitstunFrames;

	std::vector<glm::vec2> hurtboxWidthHeight; //array the size of active frames
	std::vector<glm::vec2> hurtboxPos; //array the size of active frames

	std::vector<std::vector<Hitbox>> hitboxesPerFrame;

	int animationIndex_;
	std::string animationName_;
	Hitbox hurtbox_;

	float hitPushMag; //magnitude of push when hit
	float blockPushMag; //magnitude of push when blocked

	bool handled_ = false;

	int damage = 0;

	bool jumpAttack = false;
};

