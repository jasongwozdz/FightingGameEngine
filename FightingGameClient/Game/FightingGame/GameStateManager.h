#pragma once
#include "Fighter.h"
#include "DebugDrawManager.h"

struct Arena
{
	float width;
	float depth;
	glm::vec3 pos;
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

	Entity* hurtboxDebug_[2];

	//std::vector<Attack> fighterAttacks_[2];

	CancelAttackMap cancelAttackMap_[2];

	int healthBar[2] = { 100, 100 }; 
};

class GameStateManager
{
public:
	GameStateManager& operator=(GameStateManager&& other)
	{
		std::cout << "assignment" << std::endl;

		fighterResources_.fighters_[0] = other.fighterResources_.fighters_[0];
		fighterResources_.fighters_[1] = other.fighterResources_.fighters_[1];
		other.fighterResources_.fighters_[0] = nullptr;
		other.fighterResources_.fighters_[1] = nullptr;

		fighterResources_.hurtboxDebug_[0] = other.fighterResources_.hurtboxDebug_[0];
		fighterResources_.hurtboxDebug_[1] = other.fighterResources_.hurtboxDebug_[1];
		other.fighterResources_.hurtboxDebug_[0] = nullptr;
		other.fighterResources_.hurtboxDebug_[1] = nullptr; 

		return *this;
	}

	GameStateManager() = default;

	GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, float arenaWidth, float arenaHeight);

	~GameStateManager();

	void update(float time);

	bool debug_ = true;

private:
	bool r_checkAttackCollision(const Hitbox& hitbox, const float hitboxXMin, const float hitboxXMax, const float hitboxYMin, const float hitboxYMax, glm::vec3 pos, bool left) const;

	bool checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack);

	void clampFighterOutOfBounds(Hitbox** hitboxes, Transform** transforms, Arena* arena);
	
	bool fighterCollisionCheck(Hitbox** hitboxes, Transform** transforms);

	void updateAttacks();

	void r_drawHitboxDebug(const Hitbox& hitbox, glm::vec3 pos, bool left) const;
	void drawHitboxDebug(const Hitbox& hitbox, const Fighter& fighter) const;

	void drawHealthBars();

	void drawHitboxDebug(int fighterIndex, Attack* attack);

	void checkFighterSide();

	Arena arena_;

	FighterResources fighterResources_;

	UI::UIInterface& ui_;

	DebugDrawManager* debugManager_;
};
