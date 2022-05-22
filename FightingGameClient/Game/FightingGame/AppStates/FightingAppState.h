#pragma once
#include <string>
#include <math.h>

#include "AppState.h"
#include "Events.h"
#include "BaseCamera.h"
#include "../InputHandler.h"
#include "../FighterFactory.h"
#include "../GameStateManager.h"
#include "Scene/Components/Behavior.h"

struct CommandVar;

class FightingAppState :
	public AppState
{
public:
	enum FightingAppMode
	{
		NORMAL,
		FRAMEBYFRAME
	};

	FightingAppState(std::string fighter1, std::string fighter2, DebugDrawManager* debugDrawManager, InputHandler* inputLeft, InputHandler* inputRight);
	~FightingAppState();
	virtual void enterState();
	virtual AppState* update(float deltaTime);
	void consoleCommandCallback(std::string command, CommandVar* commandVar);
	void generateArenaBackground();
	static bool isFrameByFrameModeActive();//HACK

private:
	void initScene(std::string fighterFilePath1, std::string fighterFilePath2);
	void initArena();
	void initCamera();
	void initColliderLayers();
	void initConsoleCommands();
	void checkInput();

private:
	const glm::vec3 ARENA_STARTING_POINT =  {0.0f, 0.0f, 0.0f};
	const float ARENA_WIDTH =  21.0f;
	const float ARENA_DEPTH =  21.0f;
	const float ARENA_LENGTH = 21.0f;

	Entity* fighterCamera_;
	InputHandler* inputHandlerLeft_;
	InputHandler* inputHandlerRight_;
	FighterFactory* fighterFactory_;
	GameStateManager* gameStateManager_;
	ResourceManager* resourceManager_;
	DebugDrawManager* debugDrawManager_;

	std::vector<Entity*> fighters_;

	Entity* fightingGameCamera_;
	Entity* flyCamera_;

	bool drawDebug_ = false;//should debug be drawn
	bool cursor_ = false;//is cursour disabled;
	class Scene* scene_;
	struct Arena* arena_;
	static FightingAppMode mode_;//HACK
};

#include "Scene/Components/Camera.h"

class FightingAppCameraBehavior : public BehaviorImplementationBase
{
public:
	FightingAppCameraBehavior(Entity* thisEntity, std::vector<Entity*> fighters) : 
		fighters_(fighters),
		BehaviorImplementationBase(thisEntity)
	{}

	void update()
	{
		glm::vec3 midPoint = calculateMidPoint();
		midPoint.y += HEIGHT;
		Transform& transform = entity_->getComponent<Transform>();
		transform.position_ = midPoint;
		transform.position_ += -Transform::worldForward * DISTANCE;
		transform.lookAt(midPoint - transform.position_);

		entity_->getComponent<Camera>().fovAngleInDegrees_ = calculateFov(midPoint);
	}

private:
	glm::vec3 calculateMidPoint()
	{
		glm::vec3 positions[2] = {
			fighters_[0]->getComponent<Transform>().position_,
			fighters_[1]->getComponent<Transform>().position_
		};
		//for (Entity* entity : fighters_)
		//{
		//	positions.push_back(entity->getComponent<Transform>().position_);
		//}
		glm::vec3 midPoint = positions[0] + positions[1];
		midPoint *= 0.5f;
		return(midPoint);
	}

	float calculateFov(glm::vec3 midpoint)
	{
		std::vector<glm::vec3> positions;
		for (Entity* entity : fighters_)
		{
			positions.push_back(entity->getComponent<Transform>().position_);
		}

		Transform& transform = entity_->getComponent<Transform>();
		int farthestI = 0;
		float distance = glm::distance(transform.position_, positions[farthestI]);
		if (glm::distance(transform.position_, positions[1]) > distance)
		{
			farthestI = 1;
		}

		glm::vec3 hypVec = positions[farthestI] - transform.position_;
		float hyp = glm::length(hypVec);
		float adj = glm::dot(hypVec, transform.forward());
		float angleInRads = glm::acos(adj / hyp);
		return glm::degrees(angleInRads) + FOV_PUSH;
	}

private:
#define PI 3.14159265358979323846 
	std::vector<Entity*> fighters_;

	const float DISTANCE = 20.0f;
	const float HEIGHT = 2.0f;
	const float FOV_PUSH =  10.0f;
};
