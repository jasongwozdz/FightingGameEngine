#pragma once
#include <memory>
#include <iostream>

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

private:
	virtual void onCollision(Entity* otherEnt, class BoxCollider* otherCollider) {};
	virtual void onExitCollision(Entity* otherEnt, class BoxCollider* otherCollider) {};

	virtual void onAttach() {};//called when script is attached to entity
	virtual void update() = 0;//called every frame

	friend class Behavior;
protected:
	Entity* entity_;
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

	void onCollision(Entity* otherEnt, BoxCollider* otherCollider)
	{
		behaviorImp_->onCollision(otherEnt, otherCollider);
	}

	void onExitCollision(Entity* otherEnt, BoxCollider* otherCollider)
	{
		behaviorImp_->onExitCollision(otherEnt, otherCollider);
	}

private:
	std::unique_ptr<BehaviorImplementationBase> behaviorImp_;
};
