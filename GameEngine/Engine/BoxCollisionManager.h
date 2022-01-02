#pragma once
#include <unordered_set>
#include "Scene/Components/Collider.h"
#include "Scene/Components/Transform.h"

class Scene;

class BoxCollisionManager
{
public:
	BoxCollisionManager() = default;
	~BoxCollisionManager() = default;
	void update(Scene* currentScene);
private:
	void handleCollisions(Entity* a, Entity* b);
	bool checkCollision(Transform& transformA, const BoxCollider& colliderA, Transform& transformB, const BoxCollider& colliderB);
	void drawDebug(Transform& transform, BoxCollider& collider);

	typedef std::pair<BoxCollider*, BoxCollider*> Key;
	struct KeyHash	
	{

		size_t operator()(const Key& key) const
		{

			auto boxHash = [](BoxCollider* box)
			{
				size_t hash = std::hash<float>{}(box->length_) ^
					std::hash<float>{}(box->width_)    ^
					std::hash<float>{}(box->height_)   ^
					std::hash<uint32_t>{}(box->layer_) ^
					std::hash<float>{}(box->position_[0])   ^
					std::hash<float>{}(box->position_[1])   ^
					std::hash<float>{}(box->position_[2]);

				return hash;
			};

			return boxHash(key.first) ^ boxHash(key.second);
		}
	};
	std::unordered_set<Key, KeyHash> currentCollisions_;
};
