#include "BoxCollisionManager.h"
#include "Scene/Scene.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Behavior.h"
#include "DebugDrawManager.h"
#include "Console/Console.h"

template<> BoxCollisionManager* Singleton<BoxCollisionManager>::msSingleton = 0;

BoxCollisionManager::BoxCollisionManager()
{
	msSingleton = this;
	Console::getInstance()->addBoolVar("Collision.EnableNewCollision", true);
}

void BoxCollisionManager::update(Scene* currentScene)
{
	collisionsNotHandled_ = currentCollisions_;
	std::vector<Entity*> entitys;
	currentScene->getAllEntitiesWithComponents<Collider, Transform>(entitys);
	for (int i = 0; i < ((int)entitys.size() - 1); i++)
	{
		Entity* currentEntity = entitys[i];
		for (int j = i+1; j < entitys.size(); j++)
		{
			Entity* compareEntity = entitys[j];
			if (Console::getInstance()->getBoolVar("Collision.EnableNewCollision"))
			{
				handleCollisionsNew(currentEntity, compareEntity);
			}
			else
			{
				handleCollisions(currentEntity, compareEntity);
			}
		}
	}

	//clear collisions for colliders that may not exist anymore
	for (const auto& [key, value] : collisionsNotHandled_)
	{
		Behavior* behavior = value.first->tryGetComponent<Behavior>();
		if (behavior)
		{
			behavior->onExitCollision(value.second, nullptr, nullptr);
		}

		behavior = value.second->tryGetComponent<Behavior>();
		if (behavior)
		{
			behavior->onExitCollision(value.first, nullptr, nullptr);
		}
		
		currentCollisions_.erase(key);//BUG HERE: for some reason sometimes an unhandled collision is not being erased from the map
	}
}

void BoxCollisionManager::handleCollisionsNew(Entity* entityA, Entity* entityB)
{
	Collider& colliderA = entityA->getComponent<Collider>();
	std::vector<BoxCollider>& collidersA = colliderA.colliders_;
	Transform& transformA = entityA->getComponent<Transform>();

	Collider& colliderB = entityB->getComponent<Collider>();
	std::vector<BoxCollider>& collidersB = colliderB.colliders_;
	Transform& transformB = entityB->getComponent<Transform>();
	
	struct OnCollisionParams
	{
		Behavior* behavior;
		Entity* otherEntity;
		BoxCollider thisCollider;
		BoxCollider otherCollider;
	};

	std::vector<OnCollisionParams> collisionsToHandle;

	for (int aI = 0; aI < collidersA.size(); aI++)
	{
		for (int bI = 0; bI < collidersB.size(); bI++)
		{
			BoxCollider& boxColliderA = collidersA[aI];
			BoxCollider& boxColliderB = collidersB[bI];

			drawDebug(transformA, boxColliderA);
			drawDebug(transformB, boxColliderB);

			if (!sameLayer(boxColliderA, boxColliderB))
			{
				continue;
			}

			if (isCollision(transformA, boxColliderA, transformB, boxColliderB))
			{
				Behavior* behaviorA = entityA->tryGetComponent<Behavior>();
				if (behaviorA)
				{
					collisionsToHandle.push_back({ behaviorA, entityB, boxColliderA, boxColliderB });
				}

				Behavior* behaviorB = entityB->tryGetComponent<Behavior>();
				if (behaviorB)
				{
					collisionsToHandle.push_back({ behaviorB, entityA, boxColliderB, boxColliderA });
				}
			}
		}
	}

	for (auto param : collisionsToHandle)
	{
		param.behavior->onCollision(param.otherEntity, &param.thisCollider, &param.otherCollider);
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

	for (int aI = 0; aI < collidersA.size(); aI++)
	{
		for (int bI = 0; bI < collidersB.size(); bI++)
		{
			BoxCollider& boxColliderA = collidersA[aI];
			BoxCollider& boxColliderB = collidersB[bI];

			drawDebug(transformA, boxColliderA);
			drawDebug(transformB, boxColliderB);

			Key key{ &collidersA[aI], &collidersB[bI] };

			if (!sameLayer(boxColliderA, boxColliderB))
			{
				continue;
			}

			bool collisionPairHandled = currentCollisions_.find(key) != currentCollisions_.end();
			if (collisionPairHandled)
			{
				collisionsNotHandled_.erase(key);
			}

			bool collisionDetected = isCollision(transformA, boxColliderA, transformB, boxColliderB);

			if (collisionDetected)
			{
				if (!collisionPairHandled)//collision not handled
				{
					currentCollisions_.insert({ key, std::pair(a, b) });

					//behavior callbacks
					Behavior* behavior = a->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->onCollision(b, &collidersA[aI], &collidersB[bI]);
					}
					behavior = b->tryGetComponent<Behavior>();
					if (behavior && aI < collidersA.size() && bI < collidersB.size())
					{
						behavior->onCollision(a, &collidersB[bI], &collidersA[aI]);
					}

					if (collidersA[aI].callback_)
					{
						collidersA[aI].callback_(b, &collidersA[aI], &collidersB[bI]);
					}
					if (collidersB[bI].callback_)
					{
						collidersB[bI].callback_(a, &collidersB[bI], &collidersA[aI]);
					}
				}
				else//collision handled
				{
					Behavior* behavior = a->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->whileColliding(b, &collidersA[aI], &collidersB[bI]);
					}

					behavior = b->tryGetComponent<Behavior>();
					if (behavior)
					{
						behavior->whileColliding(a, &collidersB[bI],&collidersA[aI]);
					}
				}
			}
			else if(!collisionDetected && collisionPairHandled)//colliders are no longer colliding
			{
				currentCollisions_.erase(key);

				Behavior* behavior = a->tryGetComponent<Behavior>();
				if (behavior)
				{
					behavior->onExitCollision(b, &collidersA[aI], &collidersB[bI]);
				}

				behavior = b->tryGetComponent<Behavior>();
				if (behavior)
				{
					behavior->onExitCollision(a, &collidersB[bI], &collidersA[aI]);
				}
			}
		}
	}
	
}

bool BoxCollisionManager::isCollision(Transform& transformA, const struct BoxCollider& colliderA, Transform& transformB, const struct BoxCollider& colliderB)
{
	//get world space coordinates for boxCollider
	glm::mat4 finalTransformA = transformA.calculateTransform();
	glm::mat4 finalTransformB = transformB.calculateTransform();
	glm::vec4 localPosA = { colliderA.position_ / std::abs(transformA.scale_.x), 1.0f };
	glm::vec4 worldPosA = finalTransformA * localPosA;
	glm::vec4 localPosB = { colliderB.position_ / std::abs(transformB.scale_.x), 1.0f };
	glm::vec4 worldPosB = finalTransformB * localPosB;

	float xMaxA = worldPosA.x + (colliderA.size_.x/2);
	float xMaxB = worldPosB.x + (colliderB.size_.x/2);
	float xMinA = worldPosA.x - (colliderA.size_.x/2);
	float xMinB = worldPosB.x - (colliderB.size_.x/2);

	float yMaxA = worldPosA.y + (colliderA.size_.y/2);
	float yMaxB = worldPosB.y + (colliderB.size_.y/2);
	float yMinA = worldPosA.y - (colliderA.size_.y/2);
	float yMinB = worldPosB.y - (colliderB.size_.y/2);

	float zMaxA = worldPosA.z + (colliderA.size_.z/2);
	float zMaxB = worldPosB.z + (colliderB.size_.z/2);
	float zMinA = worldPosA.z - (colliderA.size_.z/2);
	float zMinB = worldPosB.z - (colliderB.size_.z/2);

	if (((yMaxA >= yMinB && yMinA <= yMinB) || (yMaxB >= yMinA && yMinB <= yMaxA)) && //checking height
		((xMaxA >= xMinB && xMinA <= xMinB) || (xMaxB >= xMinA && xMinB <= xMaxA)) && // checking width
		((zMaxA >= zMinB && zMinA <= zMinB) || (zMaxB >= zMinA && zMinB <= zMaxA)))   // checking length
	{
		return true;
	}
	return false;
}

void BoxCollisionManager::resetColliders(std::vector<BoxCollider>& oldColliders)
{
	for (auto collider : oldColliders)
	{
		
	}
}

bool BoxCollisionManager::sameLayer(const BoxCollider & colliderA, const BoxCollider & colliderB)
{
	std::vector<int> hittableLayers = layerRules_[colliderA.layer_];
	for (int i = 0; i < hittableLayers.size(); i++)
	{
		if (hittableLayers[i] == colliderB.layer_)
		{
			return true;
		}
	}
	return false;
}

void BoxCollisionManager::drawDebug(Transform& transform, BoxCollider& collider)
{
	glm::mat4 finalTransform = transform.calculateTransform();
	glm::vec4 localPos = { collider.position_ / std::abs(transform.scale_.x), 1.0f };
	glm::vec4 worldPos = finalTransform * localPos;

	DebugDrawManager* debugDrawManager = DebugDrawManager::getSingletonPtr();

	glm::vec3 color;
	switch (collider.layer_)
	{
	case 0:
		color = { 0, 255, 0 };
		break;
	case 1:
		color = { 255, 255, 0 };
		break;
	case 2:
		color = { 255, 0, 0 };
		break;
	default:
		color = { 255, 255, 255 };
	}

	debugDrawManager->drawCube(worldPos, collider.size_, color);
}

void BoxCollisionManager::addLayerRule(int layer, int affectedLayers)
{
	auto rule = layerRules_.find(layer);
	if (rule != layerRules_.end())
	{
		rule->second.push_back(affectedLayers);
	}
	else
	{
		layerRules_.insert({ layer, std::vector<int>(1,affectedLayers) });
	}

	if (layer != affectedLayers)
	{
		rule = layerRules_.find(affectedLayers);
		if (rule != layerRules_.end())
		{
			rule->second.push_back(layer);
		}
		else
		{
			layerRules_.insert({ affectedLayers, std::vector<int>(1,layer) });
		}
	}
}

void BoxCollisionManager::clearCollisions()
{
	currentCollisions_.clear();
}

BoxCollisionManager& BoxCollisionManager::getSingleton()
{
	// TODO: insert return statement here
	if (msSingleton == 0)
	{
		msSingleton = new BoxCollisionManager();
	}
	return *msSingleton;
}

BoxCollisionManager * BoxCollisionManager::getSingletonPtr()
{
	if (msSingleton == 0)
	{
		msSingleton = new BoxCollisionManager();
	}
	return msSingleton;
}
