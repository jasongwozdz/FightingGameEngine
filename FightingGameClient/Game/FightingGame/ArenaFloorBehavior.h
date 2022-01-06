#pragma once
#include "glm/glm.hpp"

#include "Scene/Components/Behavior.h"
#include "Arena.h"

class ArenaFloorBehavior : public BehaviorImplementationBase
{
public:
	ArenaFloorBehavior(Entity* entity, Arena arena);
	~ArenaFloorBehavior();
private:
	// Inherited via BehaviorImplementationBase
	virtual void update() override;
	virtual void onCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void whileColliding(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void onExitCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);

	void drawDebug();

private:
	float pushAmount = 0.5f;
	bool drawDebug_ = true;
	Arena arena_;
	class DebugDrawManager* debugDrawManager_;
};
