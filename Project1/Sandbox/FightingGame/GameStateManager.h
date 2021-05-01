#pragma once
#include "Fighter.h"
#include "DebugDrawManager.h"

class GameStateManager
{
public:
	GameStateManager() = default;

	GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager& debugDrawManager);
	~GameStateManager();

	GameStateManager& operator=(GameStateManager&& other)
	{
		std::cout << "assignment" << std::endl;

		fighters_[0] = other.fighters_[0];
		fighters_[1] = other.fighters_[1];
		other.fighters_[0] = nullptr;
		other.fighters_[1] = nullptr;

		hurtboxDebug_[0] = other.hurtboxDebug_[0];
		hurtboxDebug_[1] = other.hurtboxDebug_[1];
		other.hurtboxDebug_[0] = nullptr;
		other.hurtboxDebug_[1] = nullptr; 

		fighterAttacks_[0] = other.fighterAttacks_[0];
		fighterAttacks_[1] = other.fighterAttacks_[1];
		other.fighterAttacks_[0] = { nullptr };
		other.fighterAttacks_[1] = { nullptr };

		return *this;
	}

	void updateAttack(Fighter& fighter, Attack& attack, Entity& hurtboxDebug, bool leftSide);

	void checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack, bool leftSide);

	//first element in the array is right side fighter second element is left side
	Fighter* fighters_[2];

	Entity* hurtboxDebug_[2];

	Attack* fighterAttacks_[2] = { {nullptr}, {nullptr} };

	Entity* pointDebug;

	void update(float time);
};
