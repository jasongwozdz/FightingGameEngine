#pragma once
#include <vector>

#include "glm/glm.hpp"
#include "Scene/Components/Collider.h"

enum ColliderLayer 
{
	PUSH_BOX = 0,
	HURT_BOX,
	HIT_BOX,
	NUM_LAYERS
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
	INVINCIBLE = 1 << 0,
	CANCELABLE = 1 << 1,
	COUNTER_HIT = 1 << 2,
	SET_VELOCITY = 1 << 3,
	EXECUTE_CANCEL = 1 << 4,
	TRANSITION_TO_AERIAL = 1 << 5
};

enum MoveState
{
	STANDING = 0,
	CROUCHING,
	JUMPING
};

struct AttackInput
{
	AttackInput(std::vector<uint8_t> attackInputIn, int attackIndex) :
		attackInput(attackInputIn)
	{};

	std::vector<uint8_t> attackInput;
	int attackIndex = 0;
	double dtBetweenAttacks; // IN MILLISECONDS
	double lastCheckTime = 0;
	float currentInput = 0;
};

struct FrameInfo
{
	FrameInfo(std::vector<BoxCollider> colliders, glm::vec3 velocity, int tags, FrameType type, int blockHeight) :
		colliders_(colliders),
		velocity_(velocity),
		frameTags_(tags),
		type_(type),
		blockHeight_(Height::LOW | Height::MID | Height::HIGH)
	{};

	FrameInfo() :
		velocity_(glm::vec3(0)),
		frameTags_(0),
		type_(STARTUP),
		blockHeight_(0),
		attackHeight_(0)
	{};

	std::vector<BoxCollider> colliders_;
	std::vector<int> cancelListIds_;//id corresponds to cancelList for this frame
	glm::vec2 velocity_;//in fighter local space
	int frameTags_;//FrameTags
	FrameType type_;
	uint8_t blockHeight_;//what heights are blocked if blocking is active this frame
	uint8_t attackHeight_;//what height is this attack if it is active this frame
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

	HitEffect() :
		blockstun_(0),
		hitstun_(0),
		freezeFrames_(0),
		pushBack_(0),
		damage_(0)
	{};

	int blockstun_;
	int hitstun_;
	int freezeFrames_;
	float damage_;
	float pushBack_;
};

struct MoveInfo
{
	HitEffect hitEffect_;
	std::vector<FrameInfo> frameData_;
	std::string name_;
	std::string animationName_;
	std::map<int, std::vector<std::string>> cancelMap_;
	MoveState moveState_;
	bool hit_ = false;//set by other fighter when this attack hits
};

struct MoveInfoComponent 
{
	MoveInfoComponent() :
		moveInfo_(nullptr)
	{};

	MoveInfoComponent(MoveInfo* moveInfo) :
		moveInfo_(moveInfo)
	{};
	
	void setCurrentMove(MoveInfo* moveInfo)
	{
		moveInfo_ = moveInfo;
	}

	HitEffect moveHit()
	{
		moveInfo_->hit_ = true;
		return moveInfo_->hitEffect_;
	}

	MoveInfo* moveInfo_;
};
