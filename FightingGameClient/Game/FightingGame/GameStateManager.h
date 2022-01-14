#pragma once
#include "Fighter/Fighter.h"
#include "Scene/Components/Collider.h"
#include "DebugDrawManager.h"
#include "Singleton.h"
#include "Arena.h"

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

class GameStateManager : public Singleton<GameStateManager>
{
public:
	GameStateManager(Entity* fighter1, Entity* fighter2, DebugDrawManager* debugDrawManager, Arena& arena);
	~GameStateManager() = default;
	void update(float deltaTime);

	static GameStateManager& getSingleton();
	static GameStateManager* getSingletonPtr();

public:
	bool debug_ = true; //should debug be drawn

private:
	//Calculates the size of each health bar before sending draw commands to the ui interface
	void drawHealthBars();
	void checkFighterSide();
	void updateTimer();
	void updateFighterTransforms();

	void handleInboundChecks();
	void handleFighterCollision();

private:
	UI::UIInterface& ui_;
	DebugDrawManager* debugManager_;
	float deltaTime_;
	struct {
		Entity* fighters_[2];
		int healthBar[2] = { 100, 100 }; 
	}fighterResources_;//stores both fighters their cancelAttackMap and healthBars

	//stores timer state
	struct {
		const int maxTime = 120;//seconds
		float startTime; // seconds
	} timerResources_;

	std::vector<Fighter*> colliding_;
	Arena& arena_;
};
