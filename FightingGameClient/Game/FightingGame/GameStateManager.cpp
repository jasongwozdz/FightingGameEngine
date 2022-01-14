#include <chrono>
#include <cmath>

#include "GameStateManager.h"
#include "EngineSettings.h"
#include "Renderer/UIInterface.h"
#include "Fighter/Attack.h"
#include "FighterStates/AttackingFighterState.h"
#include "Scene/Entity.h"

#define NUM_FIGHTERS 2

GameStateManager* GameStateManager::msSingleton = 0;


GameStateManager& GameStateManager::getSingleton()
{
	assert(msSingleton);
	return *msSingleton;
}

GameStateManager* GameStateManager::getSingletonPtr()
{
	return msSingleton;
}

GameStateManager::GameStateManager(Entity* fighter1, Entity* fighter2, DebugDrawManager* debugDrawManager, Arena& arena) :
	ui_(UI::UIInterface::getSingleton()),
	debugManager_(debugDrawManager),
	arena_(arena)
{
	fighterResources_.fighters_[0] = fighter1;
	fighterResources_.fighters_[1] = fighter2;
	msSingleton = this;
};

void GameStateManager::update(float deltaTime)
{
	deltaTime_ = deltaTime * 0.001;
	updateFighterTransforms();
	checkFighterSide();
	handleFighterCollision();
	handleInboundChecks();
	drawHealthBars();
	//updateTimer();
}


void GameStateManager::drawHealthBars()
{
	//calculate size of bars relative to the size of the current window
	float height = EngineSettings::getSingleton().windowHeight;
	float width = EngineSettings::getSingleton().windowWidth;
	float backHealthBarWidth =  width * 0.25;
	Fighter* fighter = (Fighter*)fighterResources_.fighters_[0]->getComponent<Behavior>().behaviorImp_.get();

	float healthBarWidth = ((float)fighter->health_/(float)fighter->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 60, { -width*.8, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 60, { -width*.8, -height*.1 }, { 255, 0, 0, 1 });

	fighter = (Fighter*)fighterResources_.fighters_[1]->getComponent<Behavior>().behaviorImp_.get();
	backHealthBarWidth =  width * 0.25;
	healthBarWidth = ((float)fighter->health_/(float)fighter->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 60, { -width*.2, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 60, { -width*.2, -height*.1 }, { 255, 0, 0, 1 });
}

void GameStateManager::updateTimer()
{
	const glm::vec4 TIMER_COLOR = {0, 1, 1, 1};
	const float TIMER_TEXT_SCALE = 4.0f;

	float currentTime = /*getCurrentTime() */ 0.001f;//convert to seconds
	uint32_t secondsInMatch = (uint32_t)(currentTime - timerResources_.startTime);

	glm::vec2 windowPos{};
	windowPos.y = EngineSettings::getSingleton().windowHeight * 0.05f;
	if(secondsInMatch > 9)//adjust window position based on number of digits in secondsInMatch
	{ 
		windowPos.x = EngineSettings::getSingleton().windowWidth * 0.46f;
	}
	else if (secondsInMatch > 99)
	{
		windowPos.x = EngineSettings::getSingleton().windowWidth * 0.44f;
	}
	else
	{
		windowPos.x = EngineSettings::getSingleton().windowWidth * 0.49f;
	}

	if (secondsInMatch >= timerResources_.maxTime)
	{
		if (fighterResources_.healthBar[0] < fighterResources_.healthBar[1])
		{
			std::cout << "TIMEOUT: Fighter 1 wins" << std::endl;
		}
		else if (fighterResources_.healthBar[1] < fighterResources_.healthBar[0])
		{
			std::cout << "TIMEOUT: Fighter 2 wins" << std::endl;
		}
		else
		{
			std::cout << "TIMEOUT: DRAW" << std::endl;
		}
		exit(1);
	}
	ui_.addTextToTransparentBackground(std::to_string(secondsInMatch), windowPos, TIMER_COLOR, TIMER_TEXT_SCALE);
}

void GameStateManager::updateFighterTransforms()
{
	for (int i = 0; i < 2; i++)
	{
		Fighter* fighter = Fighter::getFighterComp(fighterResources_.fighters_[i]);
		fighter->updateTransform();
	}
}

void GameStateManager::handleInboundChecks()
{
	for (int i = 0; i < 2; i++)
	{
		Transform& transform = fighterResources_.fighters_[i]->getComponent<Transform>();
		Fighter* fighter = Fighter::getFighterComp(fighterResources_.fighters_[i]);
		fighter->collidingWithWall = false;

		//collided with floor
		if (transform.position_.y < arena_.pos.y)
		{
			transform.position_.y = arena_.pos.y;
			fighter->applyGravity_ = false;
			fighter->handleFloorCollision();
		}

		//left wall collision
		if (transform.position_.x > arena_.pos.x + arena_.size.x/2)
		{
			transform.position_.x = arena_.pos.x + arena_.size.x/2;
			fighter->collidingWithWall = true;
			fighter->handleWallCollision(true);
		}

		if (transform.position_.x < arena_.pos.x - arena_.size.x/2)
		{
			transform.position_.x = arena_.pos.x - arena_.size.x/2;
			fighter->collidingWithWall = true;
			fighter->handleWallCollision(false);
		}
	}
}

void GameStateManager::handleFighterCollision()
{
	Fighter* fighter1 = Fighter::getFighterComp(fighterResources_.fighters_[0]);
	Fighter* fighter2 = Fighter::getFighterComp(fighterResources_.fighters_[1]);
	if(fighter1->colldingWithFighter && fighter2->colldingWithFighter)
	{
		Transform& fighter1Trans = fighter1->getTransform();
		Transform& fighter2Trans = fighter2->getTransform();

		float speed1 = fighter1->velocityWorldSpace_.x;
		float speed2 = fighter2->velocityWorldSpace_.x;

		Collider collider1 = fighter1->entity_->getComponent<Collider>();
		Collider collider2 = fighter2->entity_->getComponent<Collider>();

		BoxCollider fighter1Collider;
		BoxCollider fighter2Collider;

		for (BoxCollider& collider : collider1.colliders_)
		{
			if (collider.layer_ == 0)
			{
				fighter1Collider = collider;
				break;
			}
		}

		for (BoxCollider& collider : collider2.colliders_)
		{
			if (collider.layer_ == 0)
			{
				fighter2Collider = collider;
				break;
			}
		}

		//apply each fighters speed to eachother

		//calculate how close the fighters are to eachother.  Scale the offset amount based on their distance. The closer the fighters are the larger the offset will be
		float extraOffset = 0;
		glm::vec3 fighter1ColliderWorldPos = fighter1Trans.calculateTransformNoScale() * glm::vec4(fighter1Collider.position_, 1.0f);
		glm::vec3 fighter2ColliderWorldPos = fighter2Trans.calculateTransformNoScale() * glm::vec4(fighter2Collider.position_, 1.0f);

		float dist = glm::distance(fighter1ColliderWorldPos.x, fighter2ColliderWorldPos.x);
		float maxDist = (fighter1Collider.size_.x / 2) + (fighter2Collider.size_.x / 2);
		extraOffset = (maxDist - dist) * 10.0f; 

		//both checks below prevent fighter from walking inside other fighter while one of them is colliding with wall
		if (fighter2->collidingWithWall && glm::dot(fighter1->velocityWorldSpace_, fighter1Trans.forward()) >= 0.0f)
		{
			fighter1Trans.position_.x = fighter1->oldPos_.x;
		}
		if (fighter1->collidingWithWall && glm::dot(fighter2->velocityWorldSpace_, fighter2Trans.forward()) >= 0.0f)
		{
			fighter2Trans.position_.x = fighter2->oldPos_.x;
		}


		//both checks below do the following
		if (!fighter1->collidingWithWall && // don't move if you're colliding with wall
			glm::dot(fighter2->velocityWorldSpace_, fighter2Trans.forward()) >= 0.0f && //only move if other fighter is moving towards you
			glm::dot(fighter1Trans.forward(), fighter2Trans.forward()) <= 0.0f) //only move if the other fighter is facing the opposite direction as you
		{
			float offset = extraOffset;
			if (fighter1->side_ == right)
			{
				offset *= -1;
			}
			fighter1Trans.position_.x += (offset + speed2) * Scene::DeltaTimeMs;
		}
		if (!fighter2->collidingWithWall &&
			glm::dot(fighter1->velocityWorldSpace_, fighter1Trans.forward()) >= 0.0f &&
			glm::dot(fighter2Trans.forward(), fighter1Trans.forward()) <= 0.0f)
		{
			float offset = extraOffset;
			if (fighter2->side_ == right)
			{
				offset *= -1;
			}
			fighter2Trans.position_.x += (offset + speed1) * Scene::DeltaTimeMs;
		}
	}
}

void GameStateManager::checkFighterSide()
{
	Transform& transform1 = fighterResources_.fighters_[0]->getComponent<Transform>();
	Transform& transform2 = fighterResources_.fighters_[1]->getComponent<Transform>();
	Fighter* fighter1 = Fighter::getFighterComp(fighterResources_.fighters_[0]);
	Fighter* fighter2 = Fighter::getFighterComp(fighterResources_.fighters_[1]);

	if (fighter1->side_ == left  && transform1.position_.x < transform2.position_.x ||
		fighter1->side_ == right && transform1.position_.x > transform2.position_.x)
	{
		fighter1->flipSide_ = true;
	}

	if (fighter2->side_ == left  && transform2.position_.x < transform1.position_.x ||
		fighter2->side_ == right && transform2.position_.x > transform1.position_.x)
	{
		fighter2->flipSide_ = true;
	}
}

