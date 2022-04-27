#include "FighterFactory.h"
#include "ResourceManager.h"
#include "FighterFileImporter.h"
#include "Scene/Components/Collider.h"
#include "FighterSandbox/BasicFighter/LightPunch.h"
#include "Renderer/Asset/AssetInstance.h"

FighterFactory::FighterFactory(Scene& scene) :
	scene_(scene)
{}

Entity* FighterFactory::createFighterNew(const std::string & fighterFilePath, InputHandler & inputHandler)
{
	FighterFileImporter fighterFileImporter(fighterFilePath);
	ResourceManager& resourceManager = ResourceManager::getSingleton();
	AssetCreateInfo assetCreateInfo;
	assetCreateInfo.animationPath = fighterFileImporter.exportData_.modelFilePath;
	assetCreateInfo.texturePath = fighterFileImporter.exportData_.textureFilePath;
	Asset* asset = resourceManager.createAsset(assetCreateInfo);

	AnimationReturnVals ret = resourceManager.loadAnimationFile(fighterFileImporter.exportData_.modelFilePath);

	Entity* entity = scene_.addEntity("Fighter");
	AssetInstance& assetInstance = entity->addComponent<AssetInstance>(asset);

	//entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels);
	//Renderable& mesh = entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter", true);

	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	transform.drawDebugGui_ = true;
	transform.setScale(0.019f);
	entity->addComponent<MoveInfoComponent>();

	Collider& collider = entity->addComponent<Collider>(entity);
	BoxCollider boxCollider(glm::vec3(1, 2, 1), glm::vec3(0, 2, 0), 0, entity);
	collider.colliders_.push_back(boxCollider);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");

	FighterStateData idleStateData;
	idleStateData.animationName = fighterFileImporter.exportData_.idleData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.idleData.frameData, entity);
	idleStateData.frameData = fighterFileImporter.exportData_.idleData.frameData;

	FighterStateData walkingStateData;
	walkingStateData.animationName = fighterFileImporter.exportData_.walkingData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.walkingData.frameData, entity);
	walkingStateData.frameData = fighterFileImporter.exportData_.walkingData.frameData;

	FighterStateData crouchStateData;
	crouchStateData.animationName = fighterFileImporter.exportData_.crouchData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.crouchData.frameData, entity);
	crouchStateData.frameData = fighterFileImporter.exportData_.crouchData.frameData;

	FighterStateData jumpStateData;
	jumpStateData.animationName = fighterFileImporter.exportData_.jumpData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.jumpData.frameData, entity);
	jumpStateData.frameData = fighterFileImporter.exportData_.jumpData.frameData;

	FighterStateData hitStateData;
	hitStateData.animationName = fighterFileImporter.exportData_.hitData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.hitData.frameData, entity);
	hitStateData.frameData = fighterFileImporter.exportData_.hitData.frameData;

	FighterStateData blockStateData;
	blockStateData.animationName = fighterFileImporter.exportData_.blockData.animationName;
	prepareFrameData(fighterFileImporter.exportData_.blockData.frameData, entity);
	blockStateData.frameData = fighterFileImporter.exportData_.blockData.frameData;

	AttackResources standingAttacks{};
	AttackResources jumpingAttacks{};
	AttackResources crouchingAttacks{};
	std::vector<MoveInfo>& moveData = fighterFileImporter.exportData_.moveData;
	for (int i = 0; i < moveData.size(); i++)
	{
		prepareFrameData(moveData[i].frameData_, entity);
	}

	const std::vector<std::vector<uint8_t>>& inputData = fighterFileImporter.exportData_.inputs;
	for (int i = 0; i < moveData.size(); i++)
	{
		MoveInfo move = moveData[i];
		switch (move.moveState_)
		{
			case MoveState::CROUCHING:
			{
				crouchingAttacks.moves_.push_back(move);
				crouchingAttacks.inputs_.push_back({ inputData[i], (int)crouchingAttacks.moves_.size()-1 });
				crouchingAttacks.nameToAttackIndex_.insert({ move.name_, crouchingAttacks.moves_.size() });
				break;
			}
			case MoveState::STANDING:
			{
				standingAttacks.moves_.push_back(move);
				standingAttacks.inputs_.push_back({ inputData[i], (int)standingAttacks.moves_.size()-1 });
				standingAttacks.nameToAttackIndex_.insert({ move.name_, standingAttacks.moves_.size() });
				break;
			}
			case MoveState::JUMPING:
			{
				jumpingAttacks.moves_.push_back(move);
				jumpingAttacks.inputs_.push_back({ inputData[i], (int)jumpingAttacks.moves_.size()-1 });
				jumpingAttacks.nameToAttackIndex_.insert({ move.name_, jumpingAttacks.moves_.size() });
				break;
			}
		}
	}

	Fighter* fighter = new Fighter(entity, inputHandler, idleStateData, walkingStateData, crouchStateData, jumpStateData, hitStateData, blockStateData, standingAttacks, fighterFileImporter.exportData_.basePushBox);
	entity->addComponent<Behavior>(fighter);

	return entity;
}

void FighterFactory::prepareFrameData(std::vector<FrameInfo>& frameInfo, Entity* entity)
{
	for (int i = 0; i < frameInfo.size(); i++)
	{
		for (int j = 0; j < frameInfo[i].colliders_.size(); j++)
		{
			frameInfo[i].colliders_[j].entity_ = entity;
		}
	}
}

