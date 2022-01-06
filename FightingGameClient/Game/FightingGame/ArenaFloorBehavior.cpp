#include "ArenaFloorBehavior.h"
#include "DebugDrawManager.h"
#include "Fighter/Fighter.h"

ArenaFloorBehavior::ArenaFloorBehavior(Entity * entity, Arena arena) :
	arena_(arena),
	debugDrawManager_(DebugDrawManager::getSingletonPtr()),
	BehaviorImplementationBase(entity)
{}

ArenaFloorBehavior::~ArenaFloorBehavior()
{
}

void ArenaFloorBehavior::update()
{
	drawDebug();
}

void ArenaFloorBehavior::onCollision(Entity* otherEnt, BoxCollider* thisCollider ,BoxCollider* otherCollider)
{
	if (otherEnt->name_ == "Fighter")
	{
		Fighter* fighter = Fighter::getFighterComp(otherEnt);
		fighter->applyGravity_ = false;
		fighter->handleFloorCollision();

		Transform& fighterTransform = otherEnt->getComponent<Transform>();
		Transform& arenaTransform = getTransform();

		glm::vec3 localSpaceFighterPos = otherCollider->position_;
		localSpaceFighterPos.y -= otherCollider->height_ / 2;
		glm::vec3 bottomFighterPos = fighterTransform.calculateTransformNoScale() * glm::vec4(localSpaceFighterPos, 1.0f);

		glm::vec3 localSpaceArenaPos = thisCollider->position_;
		localSpaceArenaPos.y += thisCollider->width_ / 2;
		glm::vec3 topArenaPos = arenaTransform.calculateTransformNoScale() * glm::vec4(localSpaceArenaPos, 1.0f);
		topArenaPos.x = bottomFighterPos.x;
		topArenaPos.z = bottomFighterPos.z;
		
		glm::vec3 pushAmount = topArenaPos - bottomFighterPos;
		pushAmount.y += 0.1f;
		fighterTransform.position_ += pushAmount;
	}
}

void ArenaFloorBehavior::whileColliding(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
{
	if (otherEnt->name_ == "Fighter")
	{
		Fighter* fighter = (Fighter*)otherEnt->getComponent<Behavior>().behaviorImp_.get();

		Transform& fighterTransform = otherEnt->getComponent<Transform>();
		Transform& arenaTransform = getTransform();

		glm::vec3 localSpaceFighterPos = otherCollider->position_;
		localSpaceFighterPos.y -= otherCollider->height_ / 2;
		glm::vec3 bottomFighterPos = fighterTransform.calculateTransformNoScale() * glm::vec4(localSpaceFighterPos, 1.0f);
		debugDrawManager_->drawCube(bottomFighterPos, { 0.5f, 0.5f, 0.5f }, { 255, 255, 0 });

		glm::vec3 localSpaceArenaPos = thisCollider->position_;
		localSpaceArenaPos.y += thisCollider->width_ / 2;
		glm::vec3 topArenaPos = arenaTransform.calculateTransformNoScale() * glm::vec4(localSpaceArenaPos, 1.0f);
		topArenaPos.x = bottomFighterPos.x;
		topArenaPos.z = bottomFighterPos.z;
		
		glm::vec3 pushAmount = topArenaPos - bottomFighterPos;
		debugDrawManager_->drawLine(bottomFighterPos, bottomFighterPos + pushAmount, { 255, 0, 0 });
	}
	//if (otherEnt->name_ == "Fighter")
	//{
	//	Transform& fighterTransform = otherEnt->getComponent<Transform>();
	//	Transform& arenaTransform = getTransform();
	//	fighterTransform.position_ += arenaTransform.up() * pushAmount;
	//}
}

void ArenaFloorBehavior::onExitCollision(Entity * otherEnt, BoxCollider* thisCollider, BoxCollider * otherCollider)
{
}

void ArenaFloorBehavior::drawDebug()
{
	debugDrawManager_->drawGrid(arena_.pos, Transform::worldLeft, 90.0f, 21, 21, { 255, 255, 255 }); //floor

	glm::vec3 pos = arena_.pos;
	pos.z += (arena_.depth/2);
	pos.y += (arena_.width/2);
	float rotationAngle = 0.0f;
	debugDrawManager_->drawGrid(pos, Transform::worldForward, 0.0f, arena_.width, arena_.depth, { 255, 255, 255 }); //back wall

	//draw origin
	glm::vec3 origin = glm::vec3(0.0f);
	debugDrawManager_->drawLine(origin, Transform::worldForward + origin, { 0, 0, 255 });
	debugDrawManager_->drawLine(origin, Transform::worldLeft + origin, { 255, 0, 0 });
	debugDrawManager_->drawLine(origin, Transform::worldUp + origin, { 0, 255, 0 });
}
