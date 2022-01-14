#include "BoxCollisionManager.h"
#include "Scene/Scene.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Behavior.h"
#include "DebugDrawManager.h"

template<> BoxCollisionManager* Singleton<BoxCollisionManager>::msSingleton = 0;

BoxCollisionManager::BoxCollisionManager()
{
	msSingleton = this;
}

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

			if (!checkLayers(collidersA[i], collidersB[j]))
			{
				continue;
			}

			bool collisionPairHandled = currentCollisions_.find(key) != currentCollisions_.end();
			bool collisionDetected = checkCollision(transformA, collidersA[i], transformB, collidersB[j]);

			if (collisionDetected)
			{
				if (!collisionPairHandled)//collision not handled
				{
					currentCollisions_.insert(key);

					//behavior callbacks
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

					//collider specific callback
					if (collidersA[i].callback_)
					{
						collidersA[i].callback_(b, &collidersA[i], &collidersB[j]);
					}
					if (collidersB[j].callback_)
					{
						collidersB[j].callback_(a, &collidersB[j], &collidersA[i]);
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
	//glm::mat4 finalTransformA = transformA.calculateTransform();
	//glm::mat4 finalTransformB = transformB.calculateTransform();

	//glm::vec4 localMinA = {colliderA.position_.x - colliderA.size_.x/2, colliderA.position_.y - colliderA.size_.y/2, colliderA.position_.z - colliderA.size_.z/2, 1.0f};
	//glm::vec4 localMaxA = {colliderA.position_.x + colliderA.size_.x/2, colliderA.position_.y + colliderA.size_.y/2, colliderA.position_.z + colliderA.size_.z/2, 1.0f};
	//glm::vec3 worldMinA = finalTransformA * localMinA;
	//glm::vec3 worldMaxA = finalTransformA * localMaxA;

	//glm::vec4 localMinB = {colliderB.position_.x - colliderB.size_.x/2, colliderB.position_.y - colliderB.size_.y/2, colliderB.position_.z - colliderB.size_.z/2, 1.0f};
	//glm::vec4 localMaxB = {colliderB.position_.x + colliderB.size_.x/2, colliderB.position_.y + colliderB.size_.y/2, colliderB.position_.z + colliderB.size_.z/2, 1.0f};
	//glm::vec3 worldMinB = finalTransformB * localMinB;
	//glm::vec3 worldMaxB = finalTransformB * localMaxB;

	//float xMaxA = worldMaxA.x;
	//float xMaxB = worldMaxB.x;
	//float xMinA = worldMinA.x;
	//float xMinB = worldMinB.x;

	//float yMaxA = worldMaxA.y;
	//float yMaxB = worldMaxB.y;
	//float yMinA = worldMinA.y;
	//float yMinB = worldMinB.y;

	//float zMaxA = worldMaxA.z;
	//float zMaxB = worldMaxB.z;
	//float zMinA = worldMinA.z;
	//float zMinB = worldMinB.z;

	//if (((yMaxA >= yMinB && yMinA <= yMinB) || (yMaxB >= yMinA && yMinB <= yMaxA)) && //checking height
	//	((xMaxA >= xMinB && xMinA <= xMinB) || (xMaxB >= xMinA && xMinB <= xMaxA)) && // checking width
	//	((zMaxA >= zMinB && zMinA <= zMinB) || (zMaxB >= zMinA && zMinB <= zMaxA)))   // checking length
	//{
	//	return true;
	//}
	//return false;

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

bool BoxCollisionManager::checkLayers(const BoxCollider & colliderA, const BoxCollider & colliderB)
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
