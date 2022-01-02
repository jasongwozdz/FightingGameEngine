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

//Used to store what attacks can cancel into eachother.  If an attack A is cancelable in attack B  That means that if attack A is next in the fighters attackBuffer and attack B is the current attack.  Attack A will become the new attack starting from its first frame
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

class GameStateManager
{
public:
	GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, Arena& arena);

	~GameStateManager() = default;

	void update(float deltaTime);

public:
	bool debug_ = true; //should debug be drawn

	static Arena arena_;//make static so states can access the arena

private:
	bool checkAttackCollision(Fighter& fighter1, Fighter& fighter2);

	//Keeps fighters within the bounds of the arena.  Also handles when a jump reaches the floor
	void clampFighterOutOfBounds();
	
	//checks if fighters collided with eachother and pushes them based on their current speeds
	bool fighterCollisionCheck();

	//handles attack state for both fighters
	void updateAttacks();

	//Calculates the size of each health bar before sending draw commands to the ui interface
	void drawHealthBars();

	void checkFighterSide();

	void updateTimer();

	//draws grids to visualize arena bounds, fighters hitboxes, hurtboxes, and pushboxes.  Also draws x,y,z axis lines
	void drawDebug();

	void drawHitboxDebug();
	
	void drawHitbox(const glm::vec3& fighterPos, const Hitbox& hitbox, const glm::vec3& color);//helper method for drawHitboxDebug

	void drawHitbox(const Transform& transform, const Hitbox& hitbox, const glm::vec3& color);

	//Helper method that just checks if 2 hitboxes are colliding
	bool areHitboxesColliding(const glm::vec3 pos1, const Hitbox& hitbox1, const glm::vec3& pos2, const Hitbox& hitbox2);

	bool areHitboxesColliding(const Transform& transform1, const Hitbox& hitbox1, const Transform& transform2, const Hitbox& hitbox2);
private:
	UI::UIInterface& ui_;
	DebugDrawManager* debugManager_;
	float deltaTime_;
	struct {
		Fighter* fighters_[2];
		CancelAttackMap cancelAttackMap_[2];
		int healthBar[2] = { 100, 100 }; 
	}fighterResources_;//stores both fighters their cancelAttackMap and healthBars

	//stores timer state
	struct {
		const int maxTime = 120;//seconds
		float startTime; // seconds
	} timerResources_;
};
