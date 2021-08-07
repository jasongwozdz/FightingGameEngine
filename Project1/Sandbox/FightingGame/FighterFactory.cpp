#include "FighterFactory.h"
#include "ResourceManager.h"

FighterFactory::FighterFactory(Scene& scene, InputHandler& inputHandler) :
	scene_(scene),
	inputHandler_(inputHandler)
{}

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

	Fighter* fighter = new Fighter(entity, inputHandler);
	populateAttackInput("", fighter);
	populateAttacks("", fighter);

	return fighter;
}
