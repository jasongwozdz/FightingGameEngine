#pragma once
#include <glm/glm.hpp>
#include "Scene/Entity.h"
#include "Scene/Components/Behavior.h"

class AttackBase
{
public:
	AttackBase(Entity* entity, int startup, int active, int recovery);
protected:
	Entity* createNewEntity();
	void addCollider(glm::vec3 pos, glm::vec3 size, Entity* attachTo);
	void addBehavior(BehaviorImplementationBase* behavior);
	void setEntityPosition(glm::vec3 position, Entity* entity);
	void displaceEntity(glm::vec3 displacement, Entity* entity);


	int startupFrames_;
	int activeFrames_;
	int recoveryFrames_;
private:
	
};
