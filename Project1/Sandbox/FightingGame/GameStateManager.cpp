#include "GameStateManager.h"
#include "EngineSettings.h"
#include "NewRenderer/UIInterface.h"

void setHurtboxWidthHeight(Entity& hitbox, float width, float height, glm::vec3 color)
{
	std::vector<Vertex> verticies;
	glm::vec3 p1 = { 0.0f, -width/2, height/2 };
	glm::vec3 p2 = { 0.0f, width/2, height/2 };
	glm::vec3 p3 = { 0.0f, -width/2, -height/2 };
	glm::vec3 p4 = { 0.0f, width/2, -height/2 };
	verticies.push_back({ p1 , color});
	verticies.push_back({ p2 , color});
	verticies.push_back({ p3 , color});
	verticies.push_back({ p4 , color});

	std::vector<uint32_t> indices;
	indices.push_back(0);
	indices.push_back(1);

	indices.push_back(1);
	indices.push_back(3);

	indices.push_back(3);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(0);
	Renderable& mesh = hitbox.getComponent<Renderable>();
	mesh.setVertAndIndicies(verticies, indices);
}

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager& debugDrawManager, float arenaWidth, float arenaHeight) :
	ui_(UI::UIInterface::getSingleton())
{
	arena_ = { arenaWidth, arenaHeight, {0.0, 0.0, -1.75f} };

	fighterResources_.fighters_[0] = fighter1;
	fighterResources_.fighters_[1] = fighter2;
	fighterResources_.fighterAttacks_[0].resize(fighterResources_.fighters_[0]->numAttacks_);
	fighterResources_.fighterAttacks_[1].resize(fighterResources_.fighters_[1]->numAttacks_);

	Hitbox hurtbox = { 0,0, {0,0,0} };
	hurtbox.hitboxEnt_ = fighter1->entity_;
	fighterResources_.hurtboxDebug_[0] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -1, 1, -1, 1);
	fighterResources_.hurtboxDebug_[1] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -1, 1, -1, 1);
	fighterResources_.hurtboxDebug_[0]->getComponent<Renderable>().render_ = false;
	fighterResources_.hurtboxDebug_[1]->getComponent<Renderable>().render_ = false;
{
	int startup = 10;
	int active = 20;
	int recovery = 50;

	glm::vec3* hurtboxPos = new glm::vec3[active];
	hurtboxPos[0] = { 0, -1,  1 };
	hurtboxPos[1] = { 0, -1.3,  1 };
	hurtboxPos[2] = { 0, -1.3,  1 };
	hurtboxPos[3] = { 0, -1.3,  1 };
	hurtboxPos[4] = { 0, -1.5,  1 };
	hurtboxPos[5] = { 0, -1.5,  1 };
	hurtboxPos[6] = { 0, -1.5,  1 };
	hurtboxPos[7] = { 0, -1.5,  1 };
	hurtboxPos[8] = { 0, -1.5,  1 };
	hurtboxPos[9] = { 0, -1.5,  1 };
	hurtboxPos[10] = { 0, -1.5,  1 };
	hurtboxPos[11] = { 0, -1.5,  1 };
	hurtboxPos[12] = { 0, -1.5,  1 };
	hurtboxPos[13] = { 0, -1.5,  1 };
	hurtboxPos[14] = { 0, -1.5,  1 };
	hurtboxPos[15] = { 0, -1.5,  1 };
	hurtboxPos[16] = { 0, -1.5,  1 };
	hurtboxPos[17] = { 0, -1.5,  1 };
	hurtboxPos[18] = { 0, -1.5,  1 };
	hurtboxPos[19] = { 0, -1.5,  1 };

	glm::vec2* hurtboxWidthHeight = new glm::vec2[active];
	hurtboxWidthHeight[0] =  { 1, 1 };
	hurtboxWidthHeight[1] =  { 1, 1 };
	hurtboxWidthHeight[2] =  { 1, 1 };
	hurtboxWidthHeight[3] =  { 1, 1 };
	hurtboxWidthHeight[4] =  { 1, 1 };
	hurtboxWidthHeight[5] =  { 1, 1 };
	hurtboxWidthHeight[6] =  { 1, 1 };
	hurtboxWidthHeight[7] =  { 1, 1 };
	hurtboxWidthHeight[8] =  { 1, 1 };
	hurtboxWidthHeight[9] =  { 2, 1 };
	hurtboxWidthHeight[10] = { 2, 1 };
	hurtboxWidthHeight[11] = { 2, 1 };
	hurtboxWidthHeight[12] = { 2, 1 };
	hurtboxWidthHeight[13] = { 2, 1 };
	hurtboxWidthHeight[14] = { 2, 1 };
	hurtboxWidthHeight[15] = { 2, 1 };
	hurtboxWidthHeight[16] = { 2, 1 };
	hurtboxWidthHeight[17] = { 2, 1 };
	hurtboxWidthHeight[18] = { 2, 1 };
	hurtboxWidthHeight[19] = { 2, 1 };

	fighterResources_.fighterAttacks_[0][0] = Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5);
	fighterResources_.fighterAttacks_[1][0] = Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5);
}
{
	int startup = 10;
	int active = 10;
	int recovery = 50;

	glm::vec3* hurtboxPos = new glm::vec3[active];
	hurtboxPos[0] = { 0, -1,	-2 };
	hurtboxPos[1] = { 0, -1.3,  -2 };
	hurtboxPos[2] = { 0, -1.3,  -2 };
	hurtboxPos[3] = { 0, -1.3,  -2 };
	hurtboxPos[4] = { 0, -1.5,  -2 };
	hurtboxPos[5] = { 0, -1.5,  -2 };
	hurtboxPos[6] = { 0, -1.5,  -2 };
	hurtboxPos[7] = { 0, -1.5,  -2 };
	hurtboxPos[8] = { 0, -1.5,  -2 };
	hurtboxPos[9] = { 0, -1.5,  -2 };

	glm::vec2* hurtboxWidthHeight = new glm::vec2[active];
	hurtboxWidthHeight[0] = { 2, 2 };
	hurtboxWidthHeight[1] = { 2, 2 };
	hurtboxWidthHeight[2] = { 2, 2 };
	hurtboxWidthHeight[3] = { 2, 2 };
	hurtboxWidthHeight[4] = { 2, 2 };
	hurtboxWidthHeight[5] = { 2, 2 };
	hurtboxWidthHeight[6] = { 2, 2 };
	hurtboxWidthHeight[7] = { 2, 2 };
	hurtboxWidthHeight[8] = { 2, 2 };
	hurtboxWidthHeight[9] = { 2, 2 };

	fighterResources_.fighterAttacks_[0][1] = Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 50, 50, 0.1, 5);
	fighterResources_.fighterAttacks_[1][1] = Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 50, 50, 0.1, 5);
}
	fighterResources_.cancelAttackMap_[0].add(0, 1);
	fighterResources_.cancelAttackMap_[1].add(0, 1);
};

GameStateManager::~GameStateManager()
{
	delete[] fighterResources_.fighterAttacks_;
}

bool GameStateManager::checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack)
{
	Hitbox& h2 = fighter2.entity_->getComponent<Hitbox>();
	Transform& t1 = fighter1.entity_->getComponent<Transform>();
	Transform& t2 = fighter2.entity_->getComponent<Transform>();

	float fighterXMin = t2.pos_.y - (h2.width_ / 2);
	float fighterXMax = t2.pos_.y + (h2.width_ / 2);
	float fighterYMin = t2.pos_.z - (h2.height_ / 2);
	float fighterYMax = t2.pos_.z + (h2.height_ / 2);

	glm::vec3 hitboxPos = attack.hurtboxPos[attack.currentFrame - attack.startupFrames];
	if (fighter1.side_ == left) hitboxPos.y *= -1;
	glm::vec3 pos = t1.pos_ + hitboxPos;

	int currentAttackFrame = attack.currentFrame - attack.startupFrames;
	float hitboxXMin = pos.y - attack.hurtboxWidthHeight[currentAttackFrame].x / 2;
	float hitboxXMax = pos.y + attack.hurtboxWidthHeight[currentAttackFrame].x / 2;
	float hitboxYMin = pos.z - attack.hurtboxWidthHeight[currentAttackFrame].y / 2;
	float hitboxYMax = pos.z + attack.hurtboxWidthHeight[currentAttackFrame].y / 2;

	//pointDebug->getComponent<Transform>().pos_ = {0, hitboxXMin, 0 };

	if (hitboxXMin < fighterXMax && hitboxXMax > fighterXMin)
	{
		fighter2.onHit(attack.hitPushMag, attack.hitstunFrames, attack.blockstunFrames);
		attack.handled_ = true;
		std::cout << "hit" << std::endl;
		return true;
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

		if (pos.z - (hitbox.height_ / 2 ) < (arena->pos.z ))
		{
			p.pos_.z = 1 * (arena->pos.z + hitbox.height_/2);
			fighterResources_.fighters_[i]->currentYspeed_ = 0;
			if (fighterResources_.fighters_[i]->state_ == jumping)
			{
				fighterResources_.fighters_[i]->onHit(0, 0, 30);
				//fighterResources_.fighters_[i]->setOrKeepState(idle);

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

void GameStateManager::update(float time)
{
	//TODO: clean this up
	Hitbox& h1 = fighterResources_.fighters_[0]->entity_->getComponent<Hitbox>();
	Hitbox& h2 = fighterResources_.fighters_[1]->entity_->getComponent<Hitbox>();
	Hitbox* hitboxes[2] = { &h1, &h2 };

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

	FighterState state = fighterResources_.fighters_[0]->state_;
	if((state==idle || state == walking || state == blocking) && fighterResources_.fighters_[0]->currentAttack_ == -1 && fighterResources_.fighters_[0]->attackBuffer_.size() > 0)
	{
		fighterResources_.fighters_[0]->currentAttack_ = fighterResources_.fighters_[0]->attackBuffer_.front();
		fighterResources_.fighters_[0]->attackBuffer_.pop();
	}
	state = fighterResources_.fighters_[1]->state_;
	if ((state==idle || state == walking || state == blocking) && fighterResources_.fighters_[1]->currentAttack_ == -1 && fighterResources_.fighters_[1]->attackBuffer_.size() > 0)
	{
		fighterResources_.fighters_[1]->currentAttack_ = fighterResources_.fighters_[1]->attackBuffer_.front();
		fighterResources_.fighters_[1]->attackBuffer_.pop();
	}

	updateAttacks();
	checkFighterSide();
	drawHealthBars();
}

void GameStateManager::updateAttacks()
{
	int attackIndex1 = fighterResources_.fighters_[0]->currentAttack_;
	int attackIndex2 = fighterResources_.fighters_[1]->currentAttack_;
	Hitbox* h[2] = { &fighterResources_.fighters_[0]->entity_->getComponent<Hitbox>(), &fighterResources_.fighters_[1]->entity_->getComponent<Hitbox>() };
	Transform* transforms[2] = { &fighterResources_.fighters_[0]->entity_->getComponent<Transform>(), &fighterResources_.fighters_[1]->entity_->getComponent<Transform>() };
	Attack* attack = nullptr;
	if (attackIndex1 != -1)
	{
		attack = &fighterResources_.fighterAttacks_[0][attackIndex1];
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
					//std::cout << "health2: " << fighterResources_.healthBar[1] << std::endl;
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
					//std::cout << "cancel" << std::endl;
				}
			}
			//std::cout << "----active----" << std::endl;
		}

		if (attack->currentFrame > attack->startupFrames + attack->activeFrames)
		{
			fighterResources_.hurtboxDebug_[0]->getComponent<Renderable>().render_ = false;
		}

		//std::cout << "currentFrame: " << attack.currentFrame << std::endl;
		if (attack->currentFrame >= (attack->startupFrames + attack->recoveryFrames + attack->activeFrames))
		{
			//std::cout << "----end----" << std::endl;
			//std::cout << "Attack finished" << std::endl;
			//pointDebug->getComponent<Renderable>().render_ = false;
			attack->currentFrame = 0;
			attack->handled_ = false;
			fighterResources_.fighters_[0]->currentAttack_ = -1;
			return;
		}
		attack->currentFrame++;
	}

	if (attackIndex2 != -1)
	{
		attack = &fighterResources_.fighterAttacks_[1][attackIndex2];
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
					//std::cout << "health1: " << fighterResources_.healthBar[0] << std::endl;
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
					//std::cout << "cancel" << std::endl;
				}
			}
			//std::cout << "----active----" << std::endl;
		}

		if (attack->currentFrame > attack->startupFrames + attack->activeFrames)
		{
			fighterResources_.hurtboxDebug_[1]->getComponent<Renderable>().render_ = false;
		}

		//std::cout << "currentFrame: " << attack.currentFrame << std::endl;
		if (attack->currentFrame >= (attack->startupFrames + attack->recoveryFrames + attack->activeFrames))
		{
			//std::cout << "----end----" << std::endl;
			//std::cout << "Attack finished" << std::endl;
			//pointDebug->getComponent<Renderable>().render_ = false;
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
	fighterResources_.hurtboxDebug_[fighterIndex]->getComponent<Renderable>().render_ = true;
	int currentAttackFrame = attack->currentFrame - attack->startupFrames;
	glm::vec2 hurtboxDim = attack->hurtboxWidthHeight[currentAttackFrame];
	setHurtboxWidthHeight(*fighterResources_.hurtboxDebug_[0], hurtboxDim[0], hurtboxDim[1], {255, 0, 0});
	glm::vec3 hurtboxPos = attack->hurtboxPos[currentAttackFrame];
	if(fighterResources_.fighters_[fighterIndex]->side_ == left) hurtboxPos.y *= -1;
	Transform& fighterTransform = fighterResources_.fighters_[fighterIndex]->entity_->getComponent<Transform>();
	Transform& hurtboxTransform = fighterResources_.hurtboxDebug_[fighterIndex]->getComponent<Transform>();
	hurtboxTransform.pos_ = hurtboxPos + fighterTransform.pos_;
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