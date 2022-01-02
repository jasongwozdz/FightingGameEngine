#include "FighterFactory.h"
#include "ResourceManager.h"
#include "FighterFileImporter.h"

#include "Scene/Components/Collider.h"

FighterFactory::FighterFactory(Scene& scene) :
	scene_(scene)
{}

Fighter* FighterFactory::createFighter(const std::string& fighterFilePath, InputHandler& inputHandler)
{
	FighterFileImporter fighterFileImporter(fighterFilePath);
	ResourceManager& resourceManager = ResourceManager::getSingleton();
	AnimationReturnVals ret = resourceManager.loadAnimationFile(fighterFileImporter.exportData_.modelFilePath);
	TextureReturnVals texVals = resourceManager.loadTextureFile((std::string&)fighterFileImporter.exportData_.textureFilePath);
	Entity* entity = scene_.addEntity("Fighter");
	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Fighter");
	entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter", true);
	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	//transform.rotation_ = glm::rotate(glm::mat4(1.0f), fighterFileImporter.exportData_.upRotation, { 1.0f, 0.0f, 0.0f });
	//transform.rotation_ = glm::rotate(transform.rotation_, fighterFileImporter.exportData_.rightSideRotation, { 0.0f, 1.0f, 0.0f });
	transform.setScale(0.019f);
	Collider& collider = entity->addComponent<Collider>(entity);
	BoxCollider boxCollider(2, 2, 2, {0, 0, 0}, 0);
	collider.colliders_.push_back(boxCollider);
	boxCollider = { 1, 1, 5, {0, 2, 3}, 0};
	collider.colliders_.push_back(boxCollider);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");

	FighterStateData idleStateData;
	idleStateData.animationName = fighterFileImporter.exportData_.idleData.animationName;
	idleStateData.hitboxData = fighterFileImporter.exportData_.idleData.hitboxData;

	FighterStateData walkingStateData;
	walkingStateData.animationName = fighterFileImporter.exportData_.walkingData.animationName;
	walkingStateData.hitboxData = fighterFileImporter.exportData_.walkingData.hitboxData;

	FighterStateData crouchStateData;
	crouchStateData.animationName = fighterFileImporter.exportData_.crouchData.animationName;
	crouchStateData.hitboxData = fighterFileImporter.exportData_.crouchData.hitboxData;

	FighterStateData jumpStateData;
	jumpStateData.animationName = fighterFileImporter.exportData_.jumpData.animationName;
	jumpStateData.hitboxData = fighterFileImporter.exportData_.jumpData.hitboxData;

	FighterStateData hitStateData;
	hitStateData.animationName = fighterFileImporter.exportData_.hitData.animationName;
	hitStateData.hitboxData = fighterFileImporter.exportData_.hitData.hitboxData;

	FighterStateData blockStateData;
	blockStateData.animationName = fighterFileImporter.exportData_.blockData.animationName;
	blockStateData.hitboxData = fighterFileImporter.exportData_.blockData.hitboxData;

	AttackResources standingAttacks{};
	standingAttacks.attacks_ = fighterFileImporter.exportData_.attacks;
	for (int i = 0; i < fighterFileImporter.exportData_.inputData.size(); i++)
	{
		AttackInput attackInput{};
		attackInput.attackInput = fighterFileImporter.exportData_.inputData[i];
		attackInput.attackIndex = i;
		attackInput.numInputs = fighterFileImporter.exportData_.inputData[i].size();
		standingAttacks.inputs_.push_back(attackInput);
	}

	Fighter* fighter = new Fighter(entity, inputHandler, idleStateData, walkingStateData, crouchStateData, jumpStateData, hitStateData, blockStateData, standingAttacks);
	fighter->defaultHitboxes_ = fighterFileImporter.exportData_.idleData.hitboxData[0];

	fighter->setCurrentHitboxes(fighter->defaultHitboxes_);
	fighter->currentHitboxes_ = fighter->defaultHitboxes_;
	fighter->attacks_ = fighterFileImporter.exportData_.attacks;
	for(const Attack& attack : fighter->attacks_)
	{
		AnimationClip* clip = animator.getAnimationClipByName(attack.animationName_);
		if (clip)
		{
			//clip->playbackRate_ = 3;
			clip->isLooping_ = false;//set all atacking clips to non looping
		}
	}

	return fighter;
}

bool FighterFactory::populateAttackInput(std::string inputFile, Fighter* fighter)
{
	AttackInput input{};
	input.attackInput.push_back(FightingGameInput::light);
	input.attackIndex = 0;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = FightingGameInput::medium;
	input.attackIndex = 1;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = FightingGameInput::strong;
	input.attackIndex = 2;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = FightingGameInput::ultra;
	input.attackIndex = 3;
	fighter->attackInputs_.push_back(input);

	return true;
}