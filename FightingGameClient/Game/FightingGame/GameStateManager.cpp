#include <chrono>
#include "GameStateManager.h"
#include "EngineSettings.h"
#include "NewRenderer/UIInterface.h"
#include "Attack.h"
#include "FighterStates/AttackingFighterState.h"

#define NUM_FIGHTERS 2

//return in milliseconds
extern double getCurrentTime();

Arena GameStateManager::arena_{};

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, Arena& arena) :
	ui_(UI::UIInterface::getSingleton()),
	debugManager_(debugDrawManager)
{
	arena_ = arena;
	fighterResources_.fighters_[0] = fighter1;
	fighterResources_.fighters_[1] = fighter2;
	fighterResources_.cancelAttackMap_[0].cancelAttackMap_ = fighter1->cancelAttackMap_;
	fighterResources_.cancelAttackMap_[1].cancelAttackMap_ = fighter1->cancelAttackMap_;

	timerResources_.startTime = getCurrentTime() * .001;//convert to seconds
};

void GameStateManager::update(float deltaTime)
{
	deltaTime_ = deltaTime * 0.001;
	clampFighterOutOfBounds();
	fighterCollisionCheck();
	updateAttacks();
	checkFighterSide();
	drawHealthBars();
	//updateTimer();
	if(debug_)
		drawDebug();
}

bool GameStateManager::checkAttackCollision(Fighter& attackingFighter, Fighter& recievingFighter)
{
	//Grab all hurtboxes from the attacking fighter and all hitboxes from the recieving fighter and do a collision check 
	const std::vector<Hitbox>& hurtboxes = attackingFighter.currentHurtboxes_;
	const std::vector<Hitbox>& hitboxes = recievingFighter.currentHitboxes_;
	glm::vec3 attackFighterPos = attackingFighter.getPosition();
	glm::vec3 recievingFighterPos = recievingFighter.getPosition();
	for (const Hitbox& hurtbox : hurtboxes)
	{
		for (const Hitbox& hitbox : hitboxes) 
		{
			if(areHitboxesColliding(attackFighterPos, hurtbox, recievingFighterPos, hitbox))
			{
				return true;
			}
		}
	}
	return false;
}

bool GameStateManager::fighterCollisionCheck()
{
	Fighter* fighter1 = fighterResources_.fighters_[0];
	Fighter* fighter2 = fighterResources_.fighters_[1];
	Transform& fighter1Trans = fighter1->entity_->getComponent<Transform>();
	Transform& fighter2Trans = fighter2->entity_->getComponent<Transform>();

	for (const Hitbox& fighter1Pushbox : fighter1->currentPushBoxes_)
	{
		for (const Hitbox& fighter2Pushbox : fighter2->currentPushBoxes_)
		{
			//if fighters collide determine how they should be pushed.  Only do this check when fighters have finshed flipping sides.  If we don't wait for side flipping then when a fighter crossing up the other fighter while jumping into them then the jumping fighters speed will be set to 0 and all momentum will be lost.  This caused a weird effect where if a fighter slightly clips the other fighter while jumping they will drop right in front of them
			if (areHitboxesColliding(fighter1Trans.pos_, fighter1Pushbox, fighter2Trans.pos_, fighter2Pushbox) && !fighter1->flipSide_ && !fighter2->flipSide_)
			{
				float speed1 = fighter1->getXSpeed();
				float speed2 = fighter2->getXSpeed();

				//make sure that speeds are always negative;
				if (speed1 > 0)
				{
					speed1 *= -1;
				}
				else if (speed2 > 0)
				{
					speed2 *= -1;
				}
				//apply each fighters speed to eachother
				//if both fighters are moving towards eachother they will move in place
				//if only one fighter is moving towards the other then the non moving fighter will be pushed
				const float PUSH_OFFSET = 0.01f;//add a slight offset to the speed so that both fighters are pushed until they aren't touching
				
				float extraOffset = 0;//calculate how close the fighters are to eachother.  Scale the offset amount based on their distance. The closer the fighters are the larger the offset will be
				float fighter1PushboxGlobalPos = fighter1Trans.pos_.y + fighter1Pushbox.pos_.y;
				float fighter2PushboxGlobalPos = fighter2Trans.pos_.y + fighter2Pushbox.pos_.y;

				float dist = glm::distance(fighter1PushboxGlobalPos, fighter2PushboxGlobalPos);
				const float MAX_DIST = (fighter1Pushbox.width_ / 2) + (fighter2Pushbox.width_ / 2);
				extraOffset = (MAX_DIST - dist) * 10; 

				fighter1->setXSpeed(-speed2 + PUSH_OFFSET + extraOffset);
				fighter2->setXSpeed(-speed1 + PUSH_OFFSET + extraOffset);
				fighter1->updateTransform();
				fighter2->updateTransform();
				fighter1->setXSpeed(0);
				fighter2->setXSpeed(0);
			}
		}
	}
	return false;
}

void GameStateManager::clampFighterOutOfBounds()
{
	//make sure fighter is within the bounds of the Arena
	for (int fighterIndex = 0; fighterIndex < NUM_FIGHTERS; fighterIndex++)
	{
		Fighter* fighter = fighterResources_.fighters_[fighterIndex];
		Transform& fighterTransform = fighter->entity_->getComponent<Transform>();
		glm::vec3 fighterPos = fighter->getPosition();
		for (const Hitbox& pushbox : fighter->currentPushBoxes_)
		{
			//check if the fighter is colliding with the sides of the arena, if they are push them out
			if ((pushbox.width_ / 2 + fighterPos.y) > (arena_.pos.y + (arena_.width / 2)))
			{
				fighterTransform.pos_.y = (arena_.pos.y + (arena_.width / 2)) - (pushbox.width_ / 2);
				fighter->handleWallCollision(false);//collided with right side
			}

			if ((fighterPos.y - pushbox.width_ / 2) < (-arena_.width / 2 + arena_.pos.y))
			{
				fighterTransform.pos_.y = (-arena_.width / 2 + arena_.pos.y) + (pushbox.width_ / 2);
				fighter->handleWallCollision(true);//collided with left side
			}

			//check if fighter is colliding with the bottom of the arena if they are push them up
			float fighterZ = fighterPos.z;
			float pushBoxZ = pushbox.pos_.z - pushbox.height_ / 2;
			if ((fighterZ + pushBoxZ) < (arena_.pos.z ))
			{
				fighterTransform.pos_.z = (arena_.pos.z - pushBoxZ);
				fighter->handleFloorCollision();
			}
		}
	}
}

void GameStateManager::drawHealthBars()
{
	//calculate size of bars relative to the size of the current window
	float height = EngineSettings::getSingleton().windowHeight;
	float width = EngineSettings::getSingleton().windowWidth;
	float backHealthBarWidth =  width * 0.25;
	float healthBarWidth = ((float)fighterResources_.fighters_[0]->health_/(float)fighterResources_.fighters_[0]->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 60, { -width*.8, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 60, { -width*.8, -height*.1 }, { 255, 0, 0, 1 });

	backHealthBarWidth =  width * 0.25;
	healthBarWidth = ((float)fighterResources_.fighters_[1]->health_/(float)fighterResources_.fighters_[1]->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 60, { -width*.2, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 60, { -width*.2, -height*.1 }, { 255, 0, 0, 1 });
}

void GameStateManager::drawDebug()
{
	drawHitboxDebug();

	debugManager_->addLine({ 0,0,0 }, { 1,0,0 }, { 255, 0, 0 });//x
	debugManager_->addLine({ 0,0,0 }, { 0,1,0 }, { 0, 255, 0 });//y
	debugManager_->addLine({ 0,0,0 }, { 0,0,1 }, { 0, 0, 255 });//z

	debugManager_->drawGrid(arena_.pos, {0, 1, 0}, 0.0f, 21, 21, { 255, 255, 255 }); //floor

	glm::vec3 pos = arena_.pos;
	pos.x += (arena_.width / 2);
	pos.z += (arena_.width / 4);
	glm::vec3 rotationAxis = { 0, 1, 0 };
	float rotationAngle = 90.f;
	debugManager_->drawGrid(pos, rotationAxis, rotationAngle, (arena_.width/2 + 1), arena_.depth, { 255, 255, 255 }); //back wall

	pos = arena_.pos;
	rotationAxis = { 0, 1, 0 };
	rotationAngle = 0.0f;
	pos.z += (arena_.width / 2);
	debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth, { 255, 255, 255 }); //ceiling

	pos = arena_.pos;
	rotationAxis = { 1, 0, 0 };
	rotationAngle = 90.0f;
	pos.y += arena_.width / 2;
	pos.z += (arena_.width / 4);
	debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth/2+1, { 255, 255, 255 }); //left wall

	pos = arena_.pos;
	rotationAxis = { 1, 0, 0 };
	rotationAngle = 90.0f;
	pos.y -= arena_.width / 2;
	pos.z += (arena_.width / 4);
	debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth/2+1, { 255, 255, 255 }); //right wall

}

void GameStateManager::updateTimer()
{
	const glm::vec4 TIMER_COLOR = {0, 1, 1, 1};
	const float TIMER_TEXT_SCALE = 4.0f;

	float currentTime = getCurrentTime() * 0.001f;//convert to seconds
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

void GameStateManager::updateAttacks()
{
	for (uint32_t fighterIndex = 0; fighterIndex < NUM_FIGHTERS; fighterIndex++)
	{
		uint32_t opposingFighterIndex = fighterIndex ^ 1;
		Fighter* attackingFighter = fighterResources_.fighters_[fighterIndex];
		Fighter* recievingFighter = fighterResources_.fighters_[opposingFighterIndex];
		if (checkAttackCollision(*attackingFighter, *recievingFighter))
		{
			Attack* currentAttack = attackingFighter->currentAttack_;
			if (currentAttack )
			{
				if (!currentAttack->handled_)
				{
					recievingFighter->onHit(*currentAttack);
					currentAttack->handled_ = true;
				}
			}
			else
			{
				printf("ERROR: collision detected but fighter is not currently attacking");
			}
		}
	}
}

void GameStateManager::drawHitbox(const glm::vec3& fighterPos, FighterSide side, const Hitbox& hitbox, const glm::vec3& color)
{
	glm::vec3 newPos = hitbox.pos_;
	glm::vec3 pushBoxPos = { 0.0f, fighterPos.y + newPos.y, fighterPos.z + newPos.z };
	debugManager_->drawRect(pushBoxPos, color, -hitbox.width_ / 2, hitbox.width_ / 2, -hitbox.height_ / 2, hitbox.height_ / 2);
}

void GameStateManager::drawHitboxDebug()
{
	for (uint32_t fighterIndex = 0; fighterIndex < NUM_FIGHTERS; fighterIndex++)
	{
		Fighter* currentFighter = fighterResources_.fighters_[fighterIndex];
		glm::vec3 fighterPos = currentFighter->getPosition();
		for (const Hitbox& pushbox : currentFighter->currentPushBoxes_)
		{
			drawHitbox(fighterPos, currentFighter->side_, pushbox, { 0, 255, 0 });
		}
		for (const Hitbox& pushbox : currentFighter->currentHurtboxes_)
		{
			drawHitbox(fighterPos, currentFighter->side_, pushbox, { 255, 0, 0 });
		}
		for (const Hitbox& pushbox : currentFighter->currentHitboxes_)
		{
			drawHitbox(fighterPos, currentFighter->side_, pushbox, {  255, 255, 0 });
		}
	}
}

void GameStateManager::checkFighterSide()
{
	Transform& transform1 = fighterResources_.fighters_[0]->entity_->getComponent<Transform>();
	Transform& transform2 = fighterResources_.fighters_[1]->entity_->getComponent<Transform>();
	Fighter& fighter1 = *fighterResources_.fighters_[0];
	Fighter& fighter2 = *fighterResources_.fighters_[1];
	if (fighter1.side_ == left  && transform1.pos_.y > transform2.pos_.y ||
		fighter1.side_ == right && transform1.pos_.y < transform2.pos_.y)
	{
		fighter1.flipSide_ = true;
	}

	if (fighter2.side_ == left  && transform2.pos_.y > transform1.pos_.y ||
		fighter2.side_ == right && transform2.pos_.y < transform1.pos_.y)
	{
		fighter2.flipSide_ = true;
	}
}

bool GameStateManager::areHitboxesColliding(const glm::vec3 pos1, const Hitbox& hitbox1, const glm::vec3& pos2, const Hitbox& hitbox2)
{
	float hitbox1YMin = pos1.y + (hitbox1.pos_.y - hitbox1.width_ / 2);
	float hitbox1YMax = pos1.y + (hitbox1.pos_.y + hitbox1.width_ / 2);
	float hitbox1ZMin = pos1.z + (hitbox1.pos_.z - hitbox1.height_ / 2);
	float hitbox1ZMax = pos1.z + (hitbox1.pos_.z + hitbox1.height_ / 2);

	float hitbox2YMin = pos2.y + (hitbox2.pos_.y - hitbox2.width_ / 2);
	float hitbox2YMax = pos2.y + (hitbox2.pos_.y + hitbox2.width_ / 2);
	float hitbox2ZMin = pos2.z + (hitbox2.pos_.z - hitbox2.height_ / 2);
	float hitbox2ZMax = pos2.z + (hitbox2.pos_.z + hitbox2.height_ / 2);

	if (hitbox1YMin < hitbox2YMax && hitbox1YMax > hitbox2YMin && hitbox1ZMax > hitbox2ZMin && hitbox1ZMin < hitbox2ZMax)
	{
		return true;
	}
	return false;
}
