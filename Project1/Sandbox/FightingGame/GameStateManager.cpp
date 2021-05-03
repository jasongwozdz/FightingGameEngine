#include "GameStateManager.h"

//void populateAttacks(Attack** attacksArray, )
//{
//
//}

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager& debugDrawManager, float arenaWidth, float arenaHeight) :
	fighters_{ fighter1, fighter2 }
{
	const int startup = 2;
	const int active = 20;
	const int recovery = 2;

	arena_ = { arenaWidth, arenaHeight, {0.0, 0.0, -1.75f} };

	//pointDebug = debugDrawManager.addPoint({ 0,0,0 }, { 255, 255, 0 }, 1, true);
	//pointDebug->getComponent<Renderable>().render_ = false;

	Hitbox hurtbox = { 0,0, {0,0,0} };
	hurtbox.hitboxEnt_ = fighter1->entity_;
	hurtboxDebug_[0] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -1, 1, -1, 1);
	hurtboxDebug_[1] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -1, 1, -1, 1);
	hurtboxDebug_[0]->getComponent<Renderable>().render_ = false;
	hurtboxDebug_[1]->getComponent<Renderable>().render_ = false;


	glm::vec3* hurtboxPos = new glm::vec3[active];
	hurtboxPos[0] =  { 0, -1,  1 };
	hurtboxPos[1] =  { 0, -1.3,  1 };
	hurtboxPos[2] =  { 0, -1.3,  1 };
	hurtboxPos[3] =  { 0, -1.3,  1 };
	hurtboxPos[4] =  { 0, -1.5,  1 };
	hurtboxPos[5] =  { 0, -1.5,  1 };
	hurtboxPos[6] =  { 0, -1.5,  1 };
	hurtboxPos[7] =  { 0, -1.5,  1 };
	hurtboxPos[8] =  { 0, -1.5,  1 };
	hurtboxPos[9] =	 { 0, -1.5,  1 };
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
	hurtboxWidthHeight[0] =  {2, 2};
	hurtboxWidthHeight[1] =  {2, 2};
	hurtboxWidthHeight[2] =  {2, 2};
	hurtboxWidthHeight[3] =  {2, 2};
	hurtboxWidthHeight[4] =  {2, 2};
	hurtboxWidthHeight[5] =  {2, 2};
	hurtboxWidthHeight[6] =  {2, 2};
	hurtboxWidthHeight[7] =  {2, 2};
	hurtboxWidthHeight[8] =  {2, 2};
	hurtboxWidthHeight[9] =  {2, 2};
	hurtboxWidthHeight[10] = {2, 2};
	hurtboxWidthHeight[11] = {2, 2};
	hurtboxWidthHeight[12] = {2, 2};
	hurtboxWidthHeight[13] = {2, 2};
	hurtboxWidthHeight[14] = {2, 2};
	hurtboxWidthHeight[15] = {2, 2};
	hurtboxWidthHeight[16] = {2, 2};
	hurtboxWidthHeight[17] = {2, 2};
	hurtboxWidthHeight[18] = {2, 2};
	hurtboxWidthHeight[19] = {2, 2};

	Attack* attack1 = new Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 1.5);
	Attack* attack2 = new Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 1.5);

	fighterAttacks_[0] = new Attack[fighters_[0]->numAttacks_];
	fighterAttacks_[1] = new Attack[fighters_[1]->numAttacks_];

	fighterAttacks_[0] = attack1;
	fighterAttacks_[1] = attack2;

};

GameStateManager::~GameStateManager()
{
	delete[] fighterAttacks_;
}

void GameStateManager::updateAttack(Fighter& fighter1, Fighter& fighter2, Attack& attack, Entity& hurtboxDebug, bool leftSide)
{
	//std::cout << "Currently attack attack.currentFrame" << attack.currentFrame << std::endl;
	if (attack.currentFrame > attack.startupFrames && attack.currentFrame < (attack.startupFrames + attack.activeFrames))
	{
		hurtboxDebug.getComponent<Renderable>().render_ = true;
		int currentAttackFrame = attack.currentFrame - attack.startupFrames;
		glm::vec2 hurtboxDim = attack.hurtboxWidthHeight[currentAttackFrame];
		glm::vec3 hurtboxPos = attack.hurtboxPos[currentAttackFrame];
		if(leftSide)  hurtboxPos.y *= -1;
		Transform& fighterTransform = fighter1.entity_->getComponent<Transform>();
		Transform& hurtboxTransform = hurtboxDebug.getComponent<Transform>();
		hurtboxTransform.pos_ = hurtboxPos + fighterTransform.pos_;

		//pointDebug->getComponent<Renderable>().render_ = true;
		if (!attack.handled_)
		{
			checkAttackCollision(fighter1, fighter2, attack, leftSide);
		}

	}
	attack.currentFrame++;
	if (attack.currentFrame > (attack.startupFrames + attack.recoveryFrames + attack.activeFrames))
	{
		//std::cout << "Attack finished" << std::endl;
		hurtboxDebug.getComponent<Renderable>().render_ = false;
		//pointDebug->getComponent<Renderable>().render_ = false;
		attack.currentFrame = 0;
		fighter1.currentAttack_ = -1;
		attack.handled_ = false;
	}
}

void GameStateManager::checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack, bool leftSide)
{
	Hitbox& h2 = fighter2.entity_->getComponent<Hitbox>();
	Transform& t1 = fighter1.entity_->getComponent<Transform>();
	Transform& t2 = fighter2.entity_->getComponent<Transform>();

	float fighterXMin = t2.pos_.y - (h2.width_ / 2);
	float fighterXMax = t2.pos_.y + (h2.width_ / 2);
	float fighterYMin = t2.pos_.z - (h2.height_ / 2);
	float fighterYMax = t2.pos_.z + (h2.height_ / 2);

	glm::vec3 hitboxPos = attack.hurtboxPos[attack.currentFrame - attack.startupFrames];
	if (leftSide) hitboxPos.y *= -1;
	glm::vec3 pos = t1.pos_ + hitboxPos;

	float hitboxXMin = pos.y - attack.hurtboxWidthHeight[attack.currentFrame - attack.startupFrames].x/2 ;
	float hitboxXMax = pos.y + attack.hurtboxWidthHeight[attack.currentFrame - attack.startupFrames].x/2 ;
	float hitboxYMin = pos.z - attack.hurtboxWidthHeight[attack.currentFrame - attack.startupFrames].y/2 ;
	float hitboxYMax = pos.z + attack.hurtboxWidthHeight[attack.currentFrame - attack.startupFrames].y/2 ;

	//pointDebug->getComponent<Transform>().pos_ = {0, hitboxXMin, 0 };

	if (hitboxXMin < fighterXMax && hitboxXMax > fighterXMin)
	{
		fighter2.onHit(attack.hitPushMag, attack.hitstunFrames, attack.blockstunFrames);
		attack.handled_ = true;
		std::cout << "hit" << std::endl;
	}
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

		if ((hitbox.width_ / 2 + p.pos_.y) > (arena->pos.y + (arena->width / 2)))
		{
			p.pos_.y = (arena->pos.y + (arena->width / 2)) - (hitbox.width_ / 2);
		}
			
		if((p.pos_.y - hitbox.width_/2) < (-arena->width / 2 + arena->pos.y))
		{
			p.pos_.y = (-arena->width / 2 + arena->pos.y) + (hitbox.width_ / 2);
		}

		hitboxes++;
		transforms++;
	}
}

void GameStateManager::update(float time)
{
	//TODO: clean this up
	Hitbox& h1 = fighters_[0]->entity_->getComponent<Hitbox>();
	Hitbox& h2 = fighters_[1]->entity_->getComponent<Hitbox>();
	Hitbox* hitboxes[2] = { &h1, &h2 };

	Transform& t1 = fighters_[0]->entity_->getComponent<Transform>();
	Transform& t2 = fighters_[1]->entity_->getComponent<Transform>();
	Transform* transforms[2] = { &t1, &t2 };
	clampFighterOutOfBounds(hitboxes, transforms, &arena_);

	bool fightersCollided = fighterCollisionCheck(hitboxes, transforms);
	if (fightersCollided)
	{
		t2.pos_.y -= fighters_[0]->speed_;
		t1.pos_.y += fighters_[1]->speed_;
	}

	int currentAttack = fighters_[0]->currentAttack_;
	if (currentAttack != -1)
	{
		updateAttack(*fighters_[0], *fighters_[1], fighterAttacks_[0][currentAttack], *hurtboxDebug_[0], false);
	}

	currentAttack = fighters_[1]->currentAttack_;
	if (currentAttack != -1)
	{
		updateAttack(*fighters_[1], *fighters_[0], fighterAttacks_[1][currentAttack], *hurtboxDebug_[1], true);
	}
}
