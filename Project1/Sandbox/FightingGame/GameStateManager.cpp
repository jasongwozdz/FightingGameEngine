#include "GameStateManager.h"

GameStateManager::GameStateManager(Fighter* fighter1, Fighter* fighter2, DebugDrawManager& debugDrawManager) :
	fighters_{ fighter1, fighter2 }
{
	const int startup = 2;
	const int active = 20;
	const int recovery = 2;

	pointDebug = debugDrawManager.addPoint({ 0,0,0 }, { 255, 255, 0 }, 1, true);
	pointDebug->getComponent<Renderable>().render_ = false;

	Hitbox hurtbox = { 0,0, {0,0,0} };
	hurtbox.hitboxEnt_ = fighter1->entity_;
	hurtboxDebug_[0] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -2, 2, -2, 2);
	hurtboxDebug_[1] = debugDrawManager.drawRect({ 0,0,0 }, { 255, 0, 0 }, 1, true, -2, 2, -2, 2);
	hurtboxDebug_[0]->getComponent<Renderable>().render_ = false;
	hurtboxDebug_[1]->getComponent<Renderable>().render_ = false;


	glm::vec3* hurtboxPos = new glm::vec3[active];
	hurtboxPos[0] =  { 0, -1,  1 };
	hurtboxPos[1] =  { 0, -2,  1 };
	hurtboxPos[2] =  { 0, -3,  1 };
	hurtboxPos[3] =  { 0, -4,  1 };
	hurtboxPos[4] =  { 0, -5,  1 };
	hurtboxPos[5] =  { 0, -6,  1 };
	hurtboxPos[6] =  { 0, -6,  1 };
	hurtboxPos[7] =  { 0, -6,  1 };
	hurtboxPos[8] =  { 0, -6,  1 };
	hurtboxPos[9] =	 { 0, -6,  1 };
	hurtboxPos[10] = { 0, -6,  1 };
	hurtboxPos[11] = { 0, -6,  1 };
	hurtboxPos[12] = { 0, -6,  1 };
	hurtboxPos[13] = { 0, -6,  1 };
	hurtboxPos[14] = { 0, -6,  1 };
	hurtboxPos[15] = { 0, -6,  1 };
	hurtboxPos[16] = { 0, -6,  1 };
	hurtboxPos[17] = { 0, -6,  1 };
	hurtboxPos[18] = { 0, -6,  1 };
	hurtboxPos[19] = { 0, -6,  1 };

	glm::vec2* hurtboxWidthHeight = new glm::vec2[active];
	hurtboxWidthHeight[0] =  {4, 4};
	hurtboxWidthHeight[1] =  {4, 4};
	hurtboxWidthHeight[2] =  {4, 4};
	hurtboxWidthHeight[3] =  {4, 4};
	hurtboxWidthHeight[4] =  {4, 4};
	hurtboxWidthHeight[5] =  {4, 4};
	hurtboxWidthHeight[6] =  {4, 4};
	hurtboxWidthHeight[7] =  {4, 4};
	hurtboxWidthHeight[8] =  {4, 4};
	hurtboxWidthHeight[9] =  {4, 4};
	hurtboxWidthHeight[10] = {4, 4};
	hurtboxWidthHeight[11] = {4, 4};
	hurtboxWidthHeight[12] = {4, 4};
	hurtboxWidthHeight[13] = {4, 4};
	hurtboxWidthHeight[14] = {4, 4};
	hurtboxWidthHeight[15] = {4, 4};
	hurtboxWidthHeight[16] = {4, 4};
	hurtboxWidthHeight[17] = {4, 4};
	hurtboxWidthHeight[18] = {4, 4};
	hurtboxWidthHeight[19] = {4, 4};

	Attack* attack1 = new Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox);
	Attack* attack2 = new Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox);

	fighterAttacks_[0] = new Attack[fighters_[0]->numAttacks_];
	fighterAttacks_[1] = new Attack[fighters_[1]->numAttacks_];

	fighterAttacks_[0] = attack1;
	fighterAttacks_[1] = attack2;


};

GameStateManager::~GameStateManager()
{
	delete[] fighterAttacks_;
}

void GameStateManager::updateAttack(Fighter& fighter, Attack& attack, Entity& hurtboxDebug, bool leftSide)
{
	//std::cout << "Currently attack attack.currentFrame" << attack.currentFrame << std::endl;
	if (attack.currentFrame > attack.startupFrames && attack.currentFrame < (attack.startupFrames + attack.activeFrames))
	{
		hurtboxDebug.getComponent<Renderable>().render_ = true;
		int currentAttackFrame = attack.currentFrame - attack.startupFrames;
		glm::vec2 hurtboxDim = attack.hurtboxWidthHeight[currentAttackFrame];
		glm::vec3 hurtboxPos = attack.hurtboxPos[currentAttackFrame];
		if(leftSide)  hurtboxPos.y *= -1;
		Transform& fighterTransform = fighter.entity_->getComponent<Transform>();
		Transform& hurtboxTransform = hurtboxDebug.getComponent<Transform>();
		hurtboxTransform.pos_ = hurtboxPos + fighterTransform.pos_;

		pointDebug->getComponent<Renderable>().render_ = true;
	}
	attack.currentFrame++;
	if (attack.currentFrame > (attack.startupFrames + attack.recoveryFrames + attack.activeFrames))
	{
		//std::cout << "Attack finished" << std::endl;
		hurtboxDebug.getComponent<Renderable>().render_ = false;
		pointDebug->getComponent<Renderable>().render_ = false;
		attack.currentFrame = 0;
		fighter.currentAttack_ = -1;
	}
}

void GameStateManager::checkAttackCollision(Fighter& fighter1, Fighter& fighter2, Attack& attack, bool leftSide)
{
	if (attack.currentFrame > attack.startupFrames && attack.currentFrame < (attack.startupFrames + attack.activeFrames))
	{

		Hitbox& h2 = fighter2.entity_->getComponent<Hitbox>();
		Transform& t1 = fighter1.entity_->getComponent<Transform>();
		Transform& t2 = fighter2.entity_->getComponent<Transform>();

		float fighterXMin = t2.pos_.y - (h2.width_ / 2);
		float fighterXMax = t2.pos_.y + (h2.width_ / 2);
		float fighterYMin = t2.pos_.z - (h2.height_ / 2);
		float fighterYMax = t2.pos_.z + (h2.height_ / 2);

		glm::vec3 hitboxPos = attack.hurtboxPos[attack.currentFrame];
		if (leftSide) hitboxPos.y *= -1;
		glm::vec3 pos = t1.pos_ + hitboxPos;

		float hitboxXMin = pos.y - attack.hurtboxWidthHeight[attack.currentFrame].x/2 ;
		float hitboxXMax = pos.y + attack.hurtboxWidthHeight[attack.currentFrame].x/2 ;
		float hitboxYMin = pos.z - attack.hurtboxWidthHeight[attack.currentFrame].y/2 ;
		float hitboxYMax = pos.z + attack.hurtboxWidthHeight[attack.currentFrame].y/2 ;

		pointDebug->getComponent<Transform>().pos_ = {0, hitboxXMin, 0 };


		if (hitboxXMin < fighterXMax && hitboxXMax > fighterXMin)
		{
			std::cout << "hit" << std::endl;
		}
	}
}


void GameStateManager::update(float time)
{
	int currentAttack = fighters_[0]->currentAttack_;
	if (currentAttack != -1)
	{
		updateAttack(*fighters_[0], fighterAttacks_[0][currentAttack], *hurtboxDebug_[0], false);
		checkAttackCollision(*fighters_[0], *fighters_[1], fighterAttacks_[0][currentAttack], false);
		
	}
	currentAttack = fighters_[1]->currentAttack_;
	if (currentAttack != -1)
	{
		updateAttack(*fighters_[1], fighterAttacks_[1][currentAttack], *hurtboxDebug_[1], true);
		checkAttackCollision(*fighters_[1], *fighters_[0], fighterAttacks_[1][currentAttack], true);
	}

}
