#pragma once
#include "Fighter.h"
#include "DebugDrawManager.h"

struct Arena
{
	float width;
	float depth;
	glm::vec3 pos;
	Entity* backgroundEntity;
};

struct HealthBar
{
	Entity entity;
	float progress;
	std::vector<glm::vec2> verticesOuter
	{
		{0, 0}, {0, 1}, {1,0}, {1,1}
	};
	std::vector<uint32_t> indicies;
};

struct CancelAttackMap
{
	std::map<int, std::vector<int>>  cancelAttackMap_;

	void add(int currentAttack, int bufferedAttack)
	{
		auto search = cancelAttackMap_.find(currentAttack);
		if (search == cancelAttackMap_.end())
		{
			cancelAttackMap_.insert({ currentAttack, {bufferedAttack} });
		}
		else
		{
			search->second.push_back(bufferedAttack);
		}
	}

	bool isAttackCancelable(int currentAttack, int bufferedAttack)
	{
		auto attack = cancelAttackMap_.find(currentAttack);
		if (attack != cancelAttackMap_.end())
		{
			for (auto iter = attack->second.begin(); iter != attack->second.end(); iter++)
			{
				if (*iter == bufferedAttack)
				{
					return true;
				}
			}
		}
		return false;
	}
};

struct FighterResources
{
	//first element in the array is right side fighter second element is left side
	Fighter* fighters_[2];

	CancelAttackMap cancelAttackMap_[2];

	int healthBar[2] = { 100, 100 }; 
};

class GameStateManager
{
public:
	GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, Arena& arena);

	~GameStateManager();

	void update(float time);

	bool debug_ = true;

	GameStateManager& operator=(GameStateManager&& other)
	{
		fighterResources_.fighters_[0] = other.fighterResources_.fighters_[0];
		fighterResources_.fighters_[1] = other.fighterResources_.fighters_[1];
		other.fighterResources_.fighters_[0] = nullptr;
		other.fighterResources_.fighters_[1] = nullptr;

		return *this;
	}

private:
	bool checkAttackCollision(Fighter& fighter1, Fighter& fighter2);

	//Keeps fighters within the bounds of the arena.  Also handles when a jump reaches the floor
	void clampFighterOutOfBounds();
	
	bool fighterCollisionCheck();

	void updateAttacks();

	void drawHitboxDebug();

	void drawHealthBars();

	void checkFighterSide();

	void updateTimer();

private:
	Arena arena_;
	UI::UIInterface& ui_;
	DebugDrawManager* debugManager_;

	FighterResources fighterResources_;//stores both fighters and other things that are needed to keep track of gamestate such as healthbars

	struct {
		const int maxTime = 120;//seconds
		float startTime; // seconds
	} timerResources_;
};
