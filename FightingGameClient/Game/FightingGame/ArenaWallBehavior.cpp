#include "ArenaWallBehavior.h"
#include "DebugDrawManager.h"
#include "Fighter/Fighter.h"

ArenaWallBehavior::ArenaWallBehavior(Entity * entity, Arena arena, ArenaWallSide arenaWallSide) :
	debugDrawManager_(DebugDrawManager::getSingletonPtr()),
	arenaWallSide_(arenaWallSide),
	arena_(arena),
	BehaviorImplementationBase(entity)
{
	Transform& transform = entity->getComponent<Transform>();
	Transform& floorTransform = arena.entity->getComponent<Transform>();
	float rotationInDegrees;
	if (arenaWallSide_ == LEFT)
	{
		transform.position_ = floorTransform.position_;
		transform.position_.x += arena.size.x / 2;
		rotationInDegrees = 90.0f;
	}
	else
	{
		transform.position_ = floorTransform.position_;
		transform.position_.x -= arena.size.x / 2;
		rotationInDegrees = 270.0f;
	}
	transform.rotateAround(rotationInDegrees, Transform::worldForward);
}

void ArenaWallBehavior::update()
{
	drawDebug();
}

void ArenaWallBehavior::onCollision(Entity * otherEnt, BoxCollider * thisCollider, BoxCollider * otherCollider)
{
	if (otherEnt->name_ == "Fighter")
	{
		//Fighter* fighter = Fighter::getFighterComp(otherEnt);
		//fighter->handleWallCollision(arenaWallSide_ == LEFT);
		//fighter->collidingWithWall = true;
		//fighter->velocityWorldSpace_.x = 0.0f;
	}
}

void ArenaWallBehavior::whileColliding(Entity * otherEnt, BoxCollider * thisCollider, BoxCollider * otherCollider)
{
	if (otherEnt->name_ == "Fighter")
	{
		//Fighter* fighter = Fighter::getFighterComp(otherEnt);
		//fighter->handleWallCollision(arenaWallSide_ == LEFT);
		//fighter->velocityWorldSpace_.x = 0.0f;

		//Transform& fighterTransform = otherEnt->getComponent<Transform>();
		//Transform& arenaTransform = getTransform();
		//glm::vec3 localSpaceFighterPos = otherCollider->position_;

		//if (glm::dot(arenaTransform.up(), fighterTransform.forward()) > 0.0f)
		//{
		//	//facing away from the wall
		//	localSpaceFighterPos.z -= otherCollider->size_.x / 2;
		//}
		//else
		//{
		//	//facing toward the wall
		//	localSpaceFighterPos.z += otherCollider->size_.x / 2;
		//}
		//glm::vec3 sideFighterPos = fighterTransform.calculateTransformNoScale() * glm::vec4(localSpaceFighterPos, 1.0f);
		//debugDrawManager_->drawCube(sideFighterPos, {0.5f, 0.5f, 0.5f}, { 255, 0 , 0 });

		//glm::vec3 arenaWallPosLocal = thisCollider->position_;
		//if (arenaWallSide_ == LEFT)
		//{
		//	arenaWallPosLocal.x -= thisCollider->size_.x/2;
		//}
		//else
		//{
		//	arenaWallPosLocal.x += thisCollider->size_.x/2;
		//}
		//glm::vec3 arenaWallWorld = arenaTransform.calculateTransformNoScale() * glm::vec4(arenaWallPosLocal, 1.0f);
		//arenaWallWorld.y = sideFighterPos.y;
		//arenaWallWorld.z = sideFighterPos.z;
		//debugDrawManager_->drawCube(arenaWallWorld, {0.2f, 0.2f, 0.2f}, { 0, 255, 255 });
		//glm::vec3 push = arenaWallWorld - sideFighterPos;
		//glm::vec3 pos = fighterTransform.position_;
		//pos.y += 1.0f;
		//debugDrawManager_->drawLine(pos, pos + push, { 255, 0, 0 });
	}

}

void ArenaWallBehavior::onExitCollision(Entity * otherEnt, BoxCollider * thisCollider, BoxCollider * otherCollider)
{
	if (otherEnt->name_ == "Fighter")
	{
		Fighter* fighter = Fighter::getFighterComp(otherEnt);
		fighter->collidingWithWall = false;
	}
}

void ArenaWallBehavior::drawDebug()
{
	Transform& transform = entity_->getComponent<Transform>();
	glm::vec3 axisOfRotation = Transform::worldUp;
	float rotationInDegrees = 90.0f;
	glm::vec3 color = { 255.0f, 255.0f, 255.0f };
	debugDrawManager_->drawGrid(transform.position_, axisOfRotation, rotationInDegrees, arena_.size.x, arena_.size.z, color);
}
