#pragma once
#include <unordered_set>

#include "Scene/Components/Collider.h"
#include "Scene/Components/Transform.h"
#include "Singleton.h"

class Scene;

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API BoxCollisionManager : Singleton<BoxCollisionManager>
{
public:
	BoxCollisionManager();
	~BoxCollisionManager() = default;
	void update(Scene* currentScene);
	void addLayerRule(int layer, int affectedLayers);
	void clearCollisions();
	static bool checkCollision(Transform& transformA, const BoxCollider& colliderA, Transform& transformB, const BoxCollider& colliderB);

	static BoxCollisionManager& getSingleton();
	static BoxCollisionManager* getSingletonPtr();

private:
	void handleCollisions(Entity* a, Entity* b);
	//checks if collidersA layers
	bool checkLayers(const BoxCollider& colliderA, const BoxCollider& colliderB);
	void drawDebug(Transform& transform, BoxCollider& collider);

private:
	//keeps track of what layers can hit eachother
	//Key: layer, value: layers that the key layer can collide with
	std::unordered_map<int, std::vector<int>> layerRules_;

	typedef std::pair<BoxCollider*, BoxCollider*> Key;
	struct KeyHash	
	{

		size_t operator()(const Key& key) const
		{

			auto boxHash = [](BoxCollider* box)
			{
				_ASSERT(box->entity_);
				size_t hash = std::hash<float>{}(box->size_[0]) ^
					std::hash<float>{}(box->size_[1])    ^
					std::hash<float>{}(box->size_[2])   ^
					std::hash<uint32_t>{}(box->layer_) ^
					std::hash<float>{}(box->position_[0])   ^
					std::hash<float>{}(box->position_[1])   ^
					std::hash<float>{}(box->position_[2])   ^
					std::hash<Entity*>{}(box->entity_)   ^
					std::hash<BoxCollider*>{}(box);

				return hash;
			};

			return boxHash(key.first) ^ boxHash(key.second);
		}
	};
	std::unordered_map<Key, std::pair<Entity*,Entity*>, KeyHash> currentCollisions_;
	std::unordered_map<Key, std::pair<Entity*,Entity*>, KeyHash> collisionsNotHandled_;

};
