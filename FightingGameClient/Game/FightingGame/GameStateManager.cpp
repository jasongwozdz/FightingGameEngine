#include <chrono>
#include "GameStateManager.h"
#include "EngineSettings.h"
#include "NewRenderer/UIInterface.h"
#include "Attack.h"

#define NUM_FIGHTERS 2

//return in milliseconds
extern double getCurrentTime();

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, Arena& arena) :
	ui_(UI::UIInterface::getSingleton()),
	debugManager_(debugDrawManager),
	arena_(arena)
{
	fighterResources_.fighters_[0] = fighter1;
	fighterResources_.fighters_[1] = fighter2;
	fighterResources_.cancelAttackMap_[0].cancelAttackMap_ = fighter1->cancelAttackMap_;
	fighterResources_.cancelAttackMap_[1].cancelAttackMap_ = fighter1->cancelAttackMap_;

	timerResources_.startTime = getCurrentTime() * .001;//convert to seconds
};

void GameStateManager::update(float time)
{
	clampFighterOutOfBounds();
	fighterCollisionCheck();
	updateAttacks();
	checkFighterSide();
	drawHealthBars();
	updateTimer();

	if(debug_)
	{
		drawDebug();
	}
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
			if(areHitboxesColliding(fighter1Trans.pos_, fighter1Pushbox, fighter2Trans.pos_, fighter2Pushbox))
			{
				//if fighters collide determine how they should be pushed
				float speed1 = fighterResources_.fighters_[0]->speed_;
				float speed2 = fighterResources_.fighters_[1]->speed_;
				//flip the speed of whichever fighter is on the left
				if (fighterResources_.fighters_[0]->side_ == left)				{
					speed2 *= -1;
				}
				else
				{
					speed1 *= -1;
				}
				//apply each fighters speed to eachother
				//if both fighters are moving towards eachother they will move in place
				//if only one fighter is moving towards the other then the non moving fighter will be pushed
				fighter2Trans.pos_.y += speed1;
				fighter1Trans.pos_.y += speed2;
				return true;
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
			}

			if ((fighterPos.y - pushbox.width_ / 2) < (-arena_.width / 2 + arena_.pos.y))
			{
				fighterTransform.pos_.y = (-arena_.width / 2 + arena_.pos.y) + (pushbox.width_ / 2);
			}

			//check if fighter is colliding with the bottom of the arena if they are push them up
			float fighterZ = fighterPos.z;
			float pushBoxZ = pushbox.pos_.z - pushbox.height_ / 2;
			if ((fighterZ - pushBoxZ) < (arena_.pos.z ))
			{
				fighterTransform.pos_.z = (arena_.pos.z + pushBoxZ);
				fighter->currentYspeed_ = 0; //reset the current Y speed
				if (fighter->state_ == jumping)
				{
					fighter->onHit(0, 30, 30);// add 30 frames of recovery to the fighter after landing from a jump

				}
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
	float healthBarWidth = ((float)fighterResources_.healthBar[0]/(float)fighterResources_.fighters_[0]->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 60, { -width*.8, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 60, { -width*.8, -height*.1 }, { 255, 0, 0, 1 });

	backHealthBarWidth =  width * 0.25;
	healthBarWidth = ((float)fighterResources_.healthBar[1]/(float)fighterResources_.fighters_[1]->maxHealth_) * width *0.25;
	ui_.drawRect(healthBarWidth, 50, { -width*.2, -height*.1 }, { 0, 255, 0, 1 });
	ui_.drawRect(backHealthBarWidth, 50, { -width*.2, -height*.1 }, { 255, 0, 0, 1 });
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
	int attackIndex1 = fighterResources_.fighters_[0]->currentAttack_;
	int attackIndex2 = fighterResources_.fighters_[1]->currentAttack_;
	for (uint32_t fighterIndex = 0; fighterIndex < NUM_FIGHTERS; fighterIndex++)
	{
		int attackIndex = fighterResources_.fighters_[fighterIndex]->currentAttack_;
		if (attackIndex != -1)//current fighter is in the middle of an attack 
		{
			uint32_t opposingFighterIndex = fighterIndex ^ 1;
			Attack& attack = fighterResources_.fighters_[fighterIndex]->attacks_[attackIndex];
			fighterResources_.fighters_[fighterIndex]->setCurrentHitboxes(attack.hitboxesPerFrame[attack.currentFrame]);
			//Check if we are in active frames so that attack collision can be checked
			if (attack.currentFrame >= attack.startupFrames && attack.currentFrame < (attack.startupFrames + attack.activeFrames))
			{
				if (!attack.handled_)
				{
					if (checkAttackCollision(*fighterResources_.fighters_[fighterIndex], *fighterResources_.fighters_[opposingFighterIndex]))
					{
						attack.handled_ = true;
						if (fighterResources_.fighters_[opposingFighterIndex]->onHit(attack.hitPushMag, attack.hitstunFrames, attack.blockstunFrames))
						{
							fighterResources_.healthBar[opposingFighterIndex] -= attack.damage;							
						}
					}
				}
				else if(fighterResources_.fighters_[fighterIndex]->attackBuffer_.size() > 0)//check if next attack in buffer can cancel current attack 
				{
					int nextAttack = fighterResources_.fighters_[fighterIndex]->attackBuffer_.front();
					if (fighterResources_.cancelAttackMap_->isAttackCancelable(fighterResources_.fighters_[fighterIndex]->currentAttack_, nextAttack))
					{
						attack.currentFrame = 0;
						attack.handled_ = false;
						fighterResources_.fighters_[fighterIndex]->currentAttack_ = nextAttack;
						fighterResources_.fighters_[fighterIndex]->attackBuffer_.pop();
					}
				}
			}

			attack.currentFrame++;
			//attack is finished
			if (attack.currentFrame >= (attack.startupFrames + attack.recoveryFrames + attack.activeFrames))
			{
				attack.currentFrame = 0;
				attack.handled_ = false;
				fighterResources_.fighters_[fighterIndex]->currentAttack_ = -1;
				fighterResources_.fighters_[fighterIndex]->setCurrentHitboxes(fighterResources_.fighters_[fighterIndex]->defaultHitboxes_);
				return;
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
