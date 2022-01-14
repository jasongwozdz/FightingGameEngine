#pragma once
#include <vector>

#include "glm/glm.hpp"
#include "Scene/Components/Collider.h"

enum HitboxLayers
{
	PUSH_BOX = 0,
	HURT_BOX,
	HIT_BOX
};

enum Height
{
	LOW =  1 << 0,
	MID =  1 << 1,
	HIGH = 1 << 2
};

enum FrameType
{
	STARTUP,
	ACTIVE,
	RECOVERY
};

enum FrameTags 
{
	EMPTY =						 0,
	INVINCIBLE =			1 << 0,
	SPECIAL_CANCELABLE =	1 << 1,
	COUNTER_HIT =			1 << 2,
	SET_VELOCITY =			1 << 3
};

struct FrameInfo
{
	FrameInfo(std::vector<BoxCollider> colliders, glm::vec3 displacement, int tags, FrameType type, int blockHeight) :
		colliders_(colliders),
		displacement_(displacement),
		tags_(tags),
		type_(type),
		blockHeight_(Height::LOW | Height::MID | Height::HIGH)
	{};

	FrameInfo() :
		displacement_(glm::vec3(0)),
		tags_(0),
		type_(STARTUP),
		blockHeight_(0),
		attackHeight_(0)
	{};

	std::vector<BoxCollider> colliders_;
	glm::vec3 displacement_;//in fighter local space
	int tags_;
	FrameType type_;
	uint8_t blockHeight_;//what heights are blocked if blocking is active this frame
	uint8_t attackHeight_;//what height is this attack if it is active this frame
};

struct MoveInfo
{
	std::vector<FrameInfo> frameData;

};

struct HitEffect
{
	HitEffect(int blockstun, int hitstun, int freezeFrames, float pushback, float damage) :
		blockstun_(blockstun),
		hitstun_(hitstun),
		freezeFrames_(freezeFrames),
		pushBack_(pushback),
		damage_(damage)
	{};

	int blockstun_;
	int hitstun_;
	int freezeFrames_;
	float damage_;
	float pushBack_;
};
