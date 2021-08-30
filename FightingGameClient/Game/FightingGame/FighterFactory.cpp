#include "FighterFactory.h"
#include "ResourceManager.h"
#include "FighterFileImporter.h"

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
	entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter");
	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	transform.rot_ = glm::rotate(glm::mat4(1.0f), fighterFileImporter.exportData_.upRotation, { 1.0f, 0.0f, 0.0f });
	transform.rot_ = glm::rotate(transform.rot_, fighterFileImporter.exportData_.rightSideRotation, { 0.0f, 1.0f, 0.0f });
	transform.setScale(0.019f);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");

	FighterStateData idleStateData;
	idleStateData.animationName = fighterFileImporter.exportData_.idleData.animationName;
	idleStateData.hitboxData = fighterFileImporter.exportData_.idleData.hitboxData;

	FighterStateData walkingStateData;
	walkingStateData.animationName = fighterFileImporter.exportData_.walkingData.animationName;
	walkingStateData.hitboxData = fighterFileImporter.exportData_.walkingData.hitboxData;

	Fighter* fighter = new Fighter(entity, inputHandler, idleStateData, walkingStateData);
	fighter->defaultHitboxes_ = fighterFileImporter.exportData_.idleData.hitboxData[0];

	fighter->setCurrentHitboxes(fighter->defaultHitboxes_);
	fighter->currentHitboxes_ = fighter->defaultHitboxes_;
	fighter->attacks_ = fighterFileImporter.exportData_.attacks;
	for (int i = 0; i < fighterFileImporter.exportData_.inputData.size(); i++)
	{
		AttackInput attackInput{};
		attackInput.attackInput = fighterFileImporter.exportData_.inputData[i];
		attackInput.attackIndex = i;
		attackInput.numInputs = fighterFileImporter.exportData_.inputData[i].size();
		fighter->attackInputs_.push_back(attackInput);
	}


	return fighter;
}

bool FighterFactory::populateAttackInput(std::string inputFile, Fighter* fighter)
{
	AttackInput input{};
	input.attackInput.push_back(Input::light);
	input.attackIndex = 0;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = Input::medium;
	input.attackIndex = 1;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = Input::strong;
	input.attackIndex = 2;
	fighter->attackInputs_.push_back(input);

	input.attackInput[0] = Input::ultra;
	input.attackIndex = 3;
	fighter->attackInputs_.push_back(input);

	return true;
}

bool FighterFactory::populateAttacks(std::string fileLocation, Fighter* fighter)
{
	int startup = 10;
	int active = 20;
	int recovery = 50;

	std::vector<glm::vec2> hurtboxPos = 
	{
		{ -1,   1},
		{ -1.3, 1},
		{ -1.3, 1},
		{ -1.3, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1},
		{ -1.5, 1}
	};

	std::vector<glm::vec2> hurtboxWidthHeight = {
		{ 1, 1 },
		{ 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 1, 1 },
        { 2, 1 },
        { 2, 1 },
        { 2, 1 },
        { 2, 1 },
        { 2, 1 },
		{ 2, 1 },
		{ 2, 1 },
		{ 2, 1 },
		{ 2, 1 },
		{ 2, 1 },
		{ 2, 1 }
	};

	Hitbox hurtbox = { 0,0, {0,0,0}, Hitbox::HitboxLayer::Hit};
	fighter->attacks_.push_back(Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5));

	startup = 10;
	active = 10;
	recovery = 50;

	hurtboxPos = 
	{
		{ -1  , 2 },
		{ -1  , 2 },
		{ -1.3, 2 },
		{ -1.3, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 }
	};

	hurtboxWidthHeight = 
	{
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 }
	};

	fighter->attacks_.push_back(Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5));

		startup = 10;
	active = 10;
	recovery = 50;

	hurtboxPos = 
	{
		{ -1  , 2 },
		{ -1  , 2 },
		{ -1.3, 2 },
		{ -1.3, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 }
	};

	hurtboxWidthHeight = 
	{
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 }
	};

	fighter->attacks_.push_back(Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5));

		startup = 10;
	active = 10;
	recovery = 50;

	hurtboxPos = 
	{
		{ -1  , 2 },
		{ -1  , 2 },
		{ -1.3, 2 },
		{ -1.3, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 },
		{ -1.5, 2 }
	};

	hurtboxWidthHeight = 
	{
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 },
		{ 2, 2 }
	};

	fighter->attacks_.push_back(Attack(startup, active, recovery, hurtboxWidthHeight, hurtboxPos, -1, hurtbox, 5, 5, 0.1, 5));
	return true;
}

Fighter* FighterFactory::createFighter(const std::string& modelPath, const std::string& texturePath, InputHandler& inputHandler)
{
	Entity* entity = scene_.addEntity("Fighter");
	ResourceManager& resourceManager = ResourceManager::getSingleton();
	AnimationReturnVals ret = resourceManager.loadAnimationFile((std::string&)modelPath);
	TextureReturnVals texVals = resourceManager.loadTextureFile((std::string&)texturePath);

	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Fighter");
	entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter");
	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	transform.setScale(0.001f);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation(-1);

	float HITBOX_WIDTH = 2;
	float HITBOX_HEIGHT = 4;
	glm::vec3 HITBOX_POS = { 0.0f, 0.0f, 0.0f };

	Hitbox& hitbox = entity->addComponent<Hitbox>(HITBOX_WIDTH, HITBOX_HEIGHT, HITBOX_POS, Hitbox::HitboxLayer::Push);
	Hitbox child;
	child = { 2.0f, 2.0f, {0.0f, 0.5f, 0.0f}, Hitbox::HitboxLayer::Hit};
	hitbox.children_.push_back(child);
	child = { 2.0f, 2.0f, {0.0f, -1.1f, 0.0f}, Hitbox::HitboxLayer::Hit};
	hitbox.children_.push_back(child);

	Fighter* fighter = new Fighter(entity, inputHandler, {}, {});
	populateAttackInput("", fighter);
	populateAttacks("", fighter);

	return fighter;
}
