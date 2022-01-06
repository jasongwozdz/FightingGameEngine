#pragma once
#include <memory>
#include <iostream>

#include "Transform.h"

class Entity;

class BehaviorImplementationBase
{
public:
	BehaviorImplementationBase(Entity* entity) :
		entity_(entity) 
	{};

	virtual ~BehaviorImplementationBase()
	{
		std::cout << "BehaviorImplementation deleted" << std::endl; 
	};

	Entity* entity_;
private:
	virtual void onCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider) {};
	virtual void whileColliding(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider) {};
	virtual void onExitCollision(Entity* otherEnt, class BoxCollider* thisCollider, class BoxCollider* otherCollider) {};

	virtual void onAttach() {};//called when script is attached to entity
	virtual void update() = 0;//called every frame

	friend class Behavior;
protected:
	Transform& getTransform()
	{
		return entity_->getComponent<Transform>();
	}
};

class Behavior
{
public:
	Behavior(BehaviorImplementationBase* beaviorImp) :
		behaviorImp_(beaviorImp) 
	{
		behaviorImp_->onAttach();
	};

	void update()
	{
		behaviorImp_->update();
	}

	void onCollision(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
	{
		behaviorImp_->onCollision(otherEnt, thisCollider, otherCollider);
	}

	void whileColliding(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
	{
		behaviorImp_->whileColliding(otherEnt, thisCollider, otherCollider);
	}

	void onExitCollision(Entity* otherEnt, BoxCollider* thisCollider, BoxCollider* otherCollider)
	{
		behaviorImp_->onExitCollision(otherEnt, thisCollider, otherCollider);
	}

	template<typename T>
	T* getBehaviorImp()
	{
		if (dynamic_cast<T*>(behaviorImp_))
		{
			return behaviorImp_;
		}
	}

	std::unique_ptr<BehaviorImplementationBase> behaviorImp_;
};
