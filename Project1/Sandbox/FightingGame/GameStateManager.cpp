#include "GameStateManager.h"
#include "EngineSettings.h"
#include "NewRenderer/UIInterface.h"
#include "../Engine/File.h"

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager* debugDrawManager, float arenaWidth, float arenaHeight) :
	ui_(UI::UIInterface::getSingleton()),
	debugManager_(debugDrawManager)
{
	arena_ = { arenaWidth, arenaHeight, {0.0, 0.0, -1.75f} };

	fighterResources_.fighters_[0] = fighter1;
	fighterResources_.fighters_[1] = fighter2;
};

GameStateManager::~GameStateManager()
{
}

bool GameStateManager::r_checkAttackCollision(const Hitbox& hitbox, const float hitboxXMin, const float hitboxXMax, const float hitboxYMin, const float hitboxYMax, glm::vec3 pos, bool left) const
{
	//glm::vec3 newPos = pos + hitbox.pos_;
	glm::vec3 newPos = hitbox.pos_;
	if (left)
	{
		newPos.y *= -1;
	}
	newPos += pos;
	if (hitbox.layer_ == Hitbox::HitboxLayer::Hit)
	{
		float fighterXMin = newPos.y - (hitbox.width_ / 2);
		float fighterXMax = newPos.y + (hitbox.width_ / 2);
		float fighterYMin = newPos.z - (hitbox.height_ / 2);
		float fighterYMax = newPos.z + (hitbox.height_ / 2);

		if (hitboxXMin < fighterXMax && hitboxXMax > fighterXMin && hitboxYMax > fighterYMin && hitboxYMin < fighterYMax)
		{
			return true;
		}
	}

	for (int i = 0; i < hitbox.children_.size(); i++)
	{
		if (r_checkAttackCollision(hitbox.children_[i], hitboxXMin, hitboxXMax, hitboxYMin, hitboxYMax, newPos, left)) return true;
	}
	return false;
}

//Fighter1 is attacking fighter2 is defending.  Third argument is the attack that fighter1 is performing 
bool GameStateManager::checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack)
{
	Hitbox& h2 = fighter2.entity_->getComponent<Hitbox>();

	//y is x and z is y in 2d
	glm::vec3 pos1 = fighter1.getPosition();
	glm::vec3 pos2 = fighter2.getPosition();

	glm::vec2 hurtboxPos = attack.hurtboxPos[attack.currentFrame - attack.startupFrames];
	if (fighter1.side_ == left)hurtboxPos.x *= -1;
	glm::vec2 pos = { (pos1.y + hurtboxPos.x), (pos1.z + hurtboxPos.y) };

	int currentAttackFrame = attack.currentFrame - attack.startupFrames;
	float hitboxXMin = pos.x - attack.hurtboxWidthHeight[currentAttackFrame].x / 2;
	float hitboxXMax = pos.x + attack.hurtboxWidthHeight[currentAttackFrame].x / 2;
	float hitboxYMin = pos.y - attack.hurtboxWidthHeight[currentAttackFrame].y / 2;
	float hitboxYMax = pos.y + attack.hurtboxWidthHeight[currentAttackFrame].y / 2;

	if (r_checkAttackCollision(h2, hitboxXMin, hitboxXMax, hitboxYMin, hitboxYMax, pos2, fighter2.side_ == left))
	{
		attack.handled_ = true;
		return fighter2.onHit(attack.hitPushMag, attack.hitstunFrames, attack.blockstunFrames);
	}
	return false;
}

bool GameStateManager::fighterCollisionCheck(Hitbox** hitboxes, Transform** transforms)
{
	Hitbox& h1 = *(*hitboxes);

	Transform& p1 = *(*transforms);

	hitboxes++;
	transforms++;

	Hitbox& h2 = *(*hitboxes);
	Transform& p2 = *(*transforms);

	float xMax1 = p1.pos_.y + h1.width_ / 2;
	float xMin1 = p1.pos_.y - h1.width_ / 2;

	float xMax2 = p2.pos_.y + h2.width_ / 2;
	float xMin2 = p2.pos_.y - h2.width_ / 2;

	float yMax1 = p1.pos_.z + h1.height_ / 2;
	float yMin1 = p1.pos_.z - h1.height_ / 2;

	float yMax2 = p2.pos_.z + h2.height_ / 2;
	float yMin2 = p2.pos_.z - h2.height_ / 2;

	if ((xMax1 > xMin2 && xMin1 < xMax2) && (yMax1 >= yMin2 && yMax2 >= yMin1))
	{
		return true;
	}
	return false;
}

void GameStateManager::clampFighterOutOfBounds(Hitbox** hitboxes, Transform** transforms, Arena* arena)
{
	for (int i = 0; i < 2; i++)
	{
		Hitbox& hitbox = *(*hitboxes);
		Transform& p = *(*transforms);

		glm::vec3 pos = fighterResources_.fighters_[i]->getPosition();

		if ((hitbox.width_ / 2 + pos.y) > (arena->pos.y + (arena->width / 2)))
		{
			p.pos_.y = (arena->pos.y + (arena->width / 2)) - (hitbox.width_ / 2);
		}

		if ((pos.y - hitbox.width_ / 2) < (-arena->width / 2 + arena->pos.y))
		{
			p.pos_.y = (-arena->width / 2 + arena->pos.y) + (hitbox.width_ / 2);
		}

		float fighterZ = pos.z;
		if ((fighterZ - (hitbox.height_ / 2 )) < (arena->pos.z ))
		{
			p.pos_.z = (arena->pos.z + hitbox.height_/2);
			if (fighterResources_.fighters_[i]->side_ == left)
			{
				p.pos_.z *= -1;
			}
			fighterResources_.fighters_[i]->currentYspeed_ = 0;
			if (fighterResources_.fighters_[i]->state_ == jumping)
			{
				fighterResources_.fighters_[i]->onHit(0, 30, 30);

			}
		}

		hitboxes++;
		transforms++;
	}
}

void GameStateManager::drawHealthBars()
{
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

void GameStateManager::r_drawHitboxDebug(const Hitbox& hitbox, const glm::vec3 pos, bool left) const
{
	glm::vec3 newPos = hitbox.pos_;
	if (left)
	{
		newPos.y *= -1;
	}
	newPos += pos;
	glm::vec3 color;
	if (hitbox.layer_ == Hitbox::HitboxLayer::Push)
	{
		color = { 0, 255, 0 };
	}
	else if (hitbox.layer_ == Hitbox::HitboxLayer::Hurt)
	{
		color = { 255, 255, 0 };
	}
	else
	{
		color = { 255, 0, 0 };
	}
	debugManager_->drawRect(newPos, color, 0, true, -hitbox.width_ / 2, hitbox.width_ / 2, -hitbox.height_ / 2, hitbox.height_ / 2);
	for (int i = 0; i < hitbox.children_.size(); i++)
	{
		r_drawHitboxDebug(hitbox.children_[i], newPos, left);
	}
}

void GameStateManager::drawHitboxDebug(const Hitbox& hitbox, const Fighter& fighter) const
{
	glm::vec3 pos = fighter.getPosition();
	//pos.y *= -1;
	r_drawHitboxDebug(hitbox, pos, fighter.side_ == left);
}

void GameStateManager::update(float time)
{
	//TODO: clean this up
	Hitbox& h1 = fighterResources_.fighters_[0]->entity_->getComponent<Hitbox>();
	Hitbox& h2 = fighterResources_.fighters_[1]->entity_->getComponent<Hitbox>();
	Hitbox* hitboxes[2] = { &h1, &h2 };

	if (debug_)
	{
		drawHitboxDebug(h1, *fighterResources_.fighters_[0]);
		drawHitboxDebug(h2, *fighterResources_.fighters_[1]);
	}

	Transform& t1 = fighterResources_.fighters_[0]->entity_->getComponent<Transform>();
	Transform& t2 = fighterResources_.fighters_[1]->entity_->getComponent<Transform>();
	Transform* transforms[2] = { &t1, &t2 };
	clampFighterOutOfBounds(hitboxes, transforms, &arena_);

	bool fightersCollided = fighterCollisionCheck(hitboxes, transforms);
	if (fightersCollided)
	{
		float speed1 = fighterResources_.fighters_[0]->speed_;
		float speed2 = fighterResources_.fighters_[1]->speed_;
		if (fighterResources_.fighters_[0]->side_ == left)
		{
			speed2 *= -1;
		}
		else
		{
			speed1 *= -1;
		}
		t2.pos_.y += speed1;
		t1.pos_.y += speed2;
	}

	updateAttacks();
	checkFighterSide();
	drawHealthBars();

	if(debug_)
	{
		debugManager_->drawGrid(arena_.pos, {0, 1, 0}, 0.0f, 21, 21, { 255, 255, 255 }, true); //floor

		glm::vec3 pos = arena_.pos;
		pos.x += (arena_.width / 2);
		pos.z += (arena_.width / 4);
		glm::vec3 rotationAxis = { 0, 1, 0 };
		float rotationAngle = 90.f;
		debugManager_->drawGrid(pos, rotationAxis, rotationAngle, (arena_.width/2 + 1), arena_.depth, { 255, 255, 255 }, true); //back wall

		pos = arena_.pos;
		rotationAxis = { 0, 1, 0 };
		rotationAngle = 0.0f;
		pos.z += (arena_.width / 2);
		debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth, { 255, 255, 255 }, true); //ceiling

		pos = arena_.pos;
		rotationAxis = { 1, 0, 0 };
		rotationAngle = 90.0f;
		pos.y += arena_.width / 2;
		pos.z += (arena_.width / 4);
		debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth/2+1, { 255, 255, 255 }, true); //left wall

		pos = arena_.pos;
		rotationAxis = { 1, 0, 0 };
		rotationAngle = 90.0f;
		pos.y -= arena_.width / 2;
		pos.z += (arena_.width / 4);
		debugManager_->drawGrid(pos, rotationAxis, rotationAngle, arena_.width, arena_.depth/2+1, { 255, 255, 255 }, true); //right wall
	}

}

void GameStateManager::updateAttacks()
{
	int attackIndex1 = fighterResources_.fighters_[0]->currentAttack_;
	int attackIndex2 = fighterResources_.fighters_[1]->currentAttack_;
	Attack* attack = nullptr;
	if (attackIndex1 != -1)
	{
		attack = &fighterResources_.fighters_[0]->attacks_[attackIndex1];
		if (attack->currentFrame >= attack->startupFrames && attack->currentFrame < (attack->startupFrames + attack->activeFrames))
		{
			if (debug_)
			{
				drawHitboxDebug(0, attack);
			}

			if (!attack->handled_)
			{
				if (checkAttackCollision(*fighterResources_.fighters_[0], *fighterResources_.fighters_[1], *attack))
				{
					fighterResources_.healthBar[1] -= attack->damage;
				}
			}
			else if(fighterResources_.fighters_[0]->attackBuffer_.size() > 0)
			{
				int nextAttack = fighterResources_.fighters_[0]->attackBuffer_.front();
				if (fighterResources_.cancelAttackMap_->isAttackCancelable(fighterResources_.fighters_[0]->currentAttack_, nextAttack))
				{
					attack->currentFrame = 0;
					attack->handled_ = false;
					fighterResources_.fighters_[0]->currentAttack_ = nextAttack;
					fighterResources_.fighters_[0]->attackBuffer_.pop();
				}
			}
		}

		if (attack->currentFrame >= (attack->startupFrames + attack->recoveryFrames + attack->activeFrames))
		{
			attack->currentFrame = 0;
			attack->handled_ = false;
			fighterResources_.fighters_[0]->currentAttack_ = -1;
			return;
		}
		attack->currentFrame++;
	}

	if (attackIndex2 != -1)
	{
		attack = &fighterResources_.fighters_[1]->attacks_[attackIndex2];
		if (attack->currentFrame >= attack->startupFrames && attack->currentFrame < (attack->startupFrames + attack->activeFrames))
		{
			if (debug_)
			{
				drawHitboxDebug(1, attack);
			}

			if (!attack->handled_)
			{
				if (checkAttackCollision(*fighterResources_.fighters_[1], *fighterResources_.fighters_[0], *attack))
				{
					fighterResources_.healthBar[0] -= attack->damage;
				}
			}
			else if(fighterResources_.fighters_[1]->attackBuffer_.size() > 0)
			{
				int nextAttack = fighterResources_.fighters_[1]->attackBuffer_.front();
				if (fighterResources_.cancelAttackMap_->isAttackCancelable(fighterResources_.fighters_[1]->currentAttack_, nextAttack))
				{
					attack->currentFrame = 0;
					attack->handled_ = false;
					fighterResources_.fighters_[1]->currentAttack_ = nextAttack;
					fighterResources_.fighters_[1]->attackBuffer_.pop();
				}
			}
		}

		if (attack->currentFrame >= (attack->startupFrames + attack->recoveryFrames + attack->activeFrames))
		{
			attack->currentFrame = 0;
			attack->handled_ = false;
			fighterResources_.fighters_[1]->currentAttack_ = -1;
			return;
		}

		attack->currentFrame++;
	}
}

void GameStateManager::drawHitboxDebug(int fighterIndex, Attack* attack)
{
	int currentAttackFrame = attack->currentFrame - attack->startupFrames;
	glm::vec2 hurtboxDim = attack->hurtboxWidthHeight[currentAttackFrame];
	glm::vec2 hurtboxPos = attack->hurtboxPos[currentAttackFrame];

	if (fighterResources_.fighters_[fighterIndex]->side_ == left)hurtboxPos.x *= -1;
	glm::vec3 fighterPos = fighterResources_.fighters_[fighterIndex]->getPosition() ;
	glm::vec3 pos = { 0.0f, fighterPos.y + hurtboxPos.x, fighterPos.z + hurtboxPos.y };

	debugManager_->drawRect(pos, { 255, 0, 0 }, 0, false, -hurtboxDim[0] / 2, hurtboxDim[0] / 2, -hurtboxDim[1] / 2, hurtboxDim[1] / 2);
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
