#include "BoxCollisionManager.h"
#include "Scene/Scene.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Behavior.h"
#include "DebugDrawManager.h"

void BoxCollisionManager::update(Scene* currentScene)
{
	std::vector<Entity*> entitys = currentScene->getAllEntitiesWithComponents<Collider, Transform>();
	for (int i = 0; i < ((int)entitys.size() - 1); i++)
	{
		Entity* currentEntity = entitys[i];
		for (int j = i+1; j < entitys.size(); j++)
		{
			Entity* compareEntity = entitys[j];
			handleCollisions(currentEntity, compareEntity);
		}
	}
}

void BoxCollisionManager::handleCollisions(Entity* a, Entity* b)
{
	Collider& colliderA = a->getComponent<Collider>();
	std::vector<BoxCollider>& collidersA = colliderA.colliders_;
	Transform& transformA = a->getComponent<Transform>();

	Collider& colliderB = b->getComponent<Collider>();
	std::vector<BoxCollider>& collidersB = colliderB.colliders_;
	Transform& transformB = b->getComponent<Transform>();

	for (int i = 0; i < collidersA.size(); i++)
	{
		for (int j = 0; j < collidersB.size(); j++)
		{
			drawDebug(transformA, collidersA[i]);
			drawDebug(transformB, collidersB[j]);

			Key key{ &collidersA[i], &collidersB[j] };

			bool collisionPairHandled = currentCollisions_.find(key) != currentCollisions_.end();

			bool collisionDetected = checkCollision(transformA, collidersA[i], transformB, collidersB[j]);

			if (collidersA[i].layer_ == collidersB[j].layer_ && collisionDetected)
			{
				if (!collisionPairHandled)//collision not handled
				{
					currentCollisions_.insert(key);

					Behavior* behavior = a->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->onCollision(b, &collidersA[i], &collidersB[j]);
					}

					behavior = b->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->onCollision(a, &collidersB[j], &collidersA[i]);
					}
				}
				else//collision handled
				{
					Behavior* behavior = a->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->whileColliding(b, &collidersA[i], &collidersB[j]);
					}

					behavior = b->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->whileColliding(a, &collidersB[j],&collidersA[i]);
					}
				}
			}
			else if(!collisionDetected && collisionPairHandled)//colliders are no longer colliding
			{
				currentCollisions_.erase(key);

				Behavior* behavior = a->tryGetComponent<Behavior>();
				if (behavior)
				{
					behavior->onExitCollision(b, &collidersA[i], &collidersB[j]);
				}

				behavior = b->tryGetComponent<Behavior>();
				if (behavior)
				{
					behavior->onExitCollision(a, &collidersB[j], &collidersA[i]);
				}
			}
		}
	}
}

bool BoxCollisionManager::checkCollision(Transform& transformA, const struct BoxCollider& colliderA, Transform& transformB, const struct BoxCollider& colliderB)
{
	//get world space coordinates for boxCollider
	glm::mat4 finalTransformA = transformA.calculateTransform();
	glm::mat4 finalTransformB = transformB.calculateTransform();
	glm::vec4 localPosA = { colliderA.position_ / std::abs(transformA.scale_.x), 1.0f };
	glm::vec4 worldPosA = finalTransformA * localPosA;
	glm::vec4 localPosB = { colliderB.position_ / std::abs(transformA.scale_.x), 1.0f };
	glm::vec4 worldPosB = finalTransformB * localPosB;

	float xMaxA = worldPosA.x + (colliderA.width_/2);
	float xMaxB = worldPosB.x + (colliderB.width_/2);

	float xMinA = worldPosA.x - (colliderA.width_/2);
	float xMinB = worldPosB.x - (colliderB.width_/2);

	float yMaxA = worldPosA.y + (colliderA.height_/2);
	float yMaxB = worldPosB.y + (colliderB.height_/2);

	float yMinA = worldPosA.y - (colliderA.height_/2);
	float yMinB = worldPosB.y - (colliderB.height_/2);

	float zMaxA = worldPosA.z + (colliderA.length_/2);
	float zMaxB = worldPosB.z + (colliderB.length_/2);

	float zMinA = worldPosA.z - (colliderA.length_/2);
	float zMinB = worldPosB.z - (colliderB.length_/2);

	//if (yMaxA >= yMinB && yMinA <= yMaxB && xMaxA >= xMinB && xMinA <= xMaxB && zMaxA >= zMinB && zMinA <= zMaxB)
	//{
	//	return true;
	//}
	if (((yMaxA >= yMinB && yMinA <= yMinB) || (yMaxB >= yMinA && yMinB <= yMaxA)) && //checking height
		((xMaxA >= xMinB && xMinA <= xMinB) || (xMaxB >= xMinA && xMinB <= xMaxA)) && // checking width
		((zMaxA >= zMinB && zMinA <= zMinB) || (zMaxB >= zMinA && zMinB <= zMaxA)))   // checking length
	{
		return true;
	}
	return false;
}

void BoxCollisionManager::drawDebug(Transform& transform, BoxCollider& collider)
{
	glm::mat4 finalTransform = transform.calculateTransform();
	glm::vec4 localPos = { collider.position_ / std::abs(transform.scale_.x), 1.0f };
	glm::vec4 worldPos = finalTransform * localPos;

	DebugDrawManager* debugDrawManager = DebugDrawManager::getSingletonPtr();
	debugDrawManager->drawCube(worldPos, { collider.length_, collider.width_, collider.height_ }, { 0, 255, 0 });
}


