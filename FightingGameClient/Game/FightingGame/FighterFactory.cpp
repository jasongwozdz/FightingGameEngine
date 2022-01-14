#include "FighterFactory.h"
#include "ResourceManager.h"
#include "FighterFileImporter.h"
#include "Scene/Components/Collider.h"
#include "FighterSandbox/BasicFighter/LightPunch.h"

FighterFactory::FighterFactory(Scene& scene) :
	scene_(scene)
{}

Entity* FighterFactory::createFighterNew(const std::string & fighterFilePath, InputHandler & inputHandler)
{
	FighterFileImporter fighterFileImporter(fighterFilePath);
	ResourceManager& resourceManager = ResourceManager::getSingleton();
	AnimationReturnVals ret = resourceManager.loadAnimationFile(fighterFileImporter.exportData_.modelFilePath);
	TextureReturnVals texVals = resourceManager.loadTextureFile((std::string&)fighterFileImporter.exportData_.textureFilePath);

	Entity* entity = scene_.addEntity("Fighter");
	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Fighter");
	Renderable& mesh = entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter", true);
	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	transform.setScale(0.019f);
	Collider& collider = entity->addComponent<Collider>(entity);

	BoxCollider boxCollider(glm::vec3(1, 2, 1), glm::vec3(0, 2, 0), 0);
	collider.colliders_.push_back(boxCollider);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");

	FighterStateData idleStateData;
	idleStateData.animationName = fighterFileImporter.exportData_.idleData.animationName;
	idleStateData.frameData = fighterFileImporter.exportData_.idleData.frameData;

	FighterStateData walkingStateData;
	walkingStateData.animationName = fighterFileImporter.exportData_.walkingData.animationName;
	walkingStateData.frameData = fighterFileImporter.exportData_.walkingData.frameData;

	FighterStateData crouchStateData;
	crouchStateData.animationName = fighterFileImporter.exportData_.crouchData.animationName;
	crouchStateData.frameData = fighterFileImporter.exportData_.crouchData.frameData;

	FighterStateData jumpStateData;
	jumpStateData.animationName = fighterFileImporter.exportData_.jumpData.animationName;
	jumpStateData.frameData = fighterFileImporter.exportData_.jumpData.frameData;

	FighterStateData hitStateData;
	hitStateData.animationName = fighterFileImporter.exportData_.hitData.animationName;
	hitStateData.frameData = fighterFileImporter.exportData_.hitData.frameData;

	FighterStateData blockStateData;
	blockStateData.animationName = fighterFileImporter.exportData_.blockData.animationName;
	blockStateData.frameData = fighterFileImporter.exportData_.blockData.frameData;

	AttackResources standingAttacks{};
	std::vector<FighterFileImporter::AttackData> attackData = fighterFileImporter.exportData_.attackData;
	for (int i = 0; i < attackData.size(); i++)
	{
		FighterFileImporter::AttackData currentAttack = attackData[i];
		switch (i)
		{
		case 0:
			HitEffect hitEffect(currentAttack.blockstun_, currentAttack.hitstun_, currentAttack.freezeFrames_, currentAttack.push_, currentAttack.damage_);

			standingAttacks.newAttacks_.push_back(new AttackBase(entity, currentAttack.startupFrames_, currentAttack.activeFrames_, currentAttack.recoveryFrames_, hitEffect,currentAttack.animationName_, currentAttack.frameData_));

			AttackInput currentAttackInput(currentAttack.inputs_, i);
			standingAttacks.inputs_.push_back(currentAttackInput);
			break;
		}
	}

	Fighter* fighter = new Fighter(entity, inputHandler, idleStateData, walkingStateData, crouchStateData, jumpStateData, hitStateData, blockStateData, standingAttacks, fighterFileImporter.exportData_.basePushBox);
	entity->addComponent<Behavior>(fighter);

	return entity;
}
