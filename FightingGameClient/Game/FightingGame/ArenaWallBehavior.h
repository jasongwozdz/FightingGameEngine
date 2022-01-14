#pragma once
#include "Scene/Components/Behavior.h"
#include "glm/glm.hpp"
#include "Arena.h"

class ArenaWallBehavior : public BehaviorImplementationBase
{
public:
	enum ArenaWallSide
	{
		LEFT,
		RIGHT
	} arenaWallSide_;

	ArenaWallBehavior(Entity* entity, Arena arena, ArenaWallSide arenaWallSide);
	~ArenaWallBehavior() = default;
public:
private:
	// Inherited via BehaviorImplementationBase
	virtual void update() override;
	virtual void onCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void whileColliding(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	virtual void onExitCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider);
	void setPosition();
	void drawDebug();

private:
	float pushAmount = 0.5f;
	bool drawDebug_ = true;
	struct Arena arena_;
	class DebugDrawManager* debugDrawManager_;
};

