#pragma once
#include <glm/glm.hpp>
#include "Scene/Entity.h"
#include "Scene/Components/Behavior.h"
#include "Scene/Components/Collider.h"
#include "AttackTypes.h"

class AttackBase
{
public:
	AttackBase(Entity* initatingEntity, int startup, int active, int recovery, HitEffect hitEffect, std::string animationName, std::vector<FrameInfo> frameData);
	~AttackBase() = default;

	virtual void initateAttack();
	//return true when finished
	virtual bool updateAttack();

protected:
	Entity* createNewEntity(std::string name);
	void addCollider(Entity* attachTo, glm::vec3 pos, glm::vec3 size, int layer);
	void addBehavior(Entity* entity, BehaviorImplementationBase* behavior);
	void setEntityPosition(glm::vec3 position, Entity* entity);
	void displaceEntity(glm::vec3 displacement, Entity* entity);
	void setAnimation(Entity* entity, std::string animationName);
	void setAnimationTime();
	void setColliders();
	int getAnimFrame();
	void readFrameTags();

protected:
	std::string AttackName;
	class Scene* scene_;
	class Entity* entity_;
	HitEffect hitEffect_;
	std::vector<FrameInfo> frameData_;
	std::string animationName_;
	int currentFrameIndex_;
	int startupFrames_;
	int activeFrames_;
	int recoveryFrames_;
	int totalFrames_;
	float damage_;
};
