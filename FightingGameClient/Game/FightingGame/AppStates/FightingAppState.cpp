#include "FightingAppState.h"
#include "ResourceManager.h"
#include "Scene/Components/Camera.h"
#include "Scene/Behaviors/ControllableCameraBehavior.h"
#include "../Arena.h"
#include "../ArenaFloorBehavior.h"
#include "../ArenaWallBehavior.h"
#include "Input.h"
#include "../FighterSandbox/AttackTypes.h"

FightingAppState::FightingAppState(std::string fighter1, std::string fighter2, DebugDrawManager* debugDrawManager, InputHandler* inputLeft, InputHandler* inputRight) : 
	scene_(Scene::getSingletonPtr()),
	inputHandlerLeft_(inputLeft),
	inputHandlerRight_(inputRight),
	debugDrawManager_(debugDrawManager)
{
	fighter1 = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\Fighter1-NewAttack.fgAnim";
	fighter2 = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\Fighter1-NewAttack.fgAnim";
	inputHandlerLeft_->clearInputQueue();
	inputHandlerRight_->clearInputQueue();
	fighterFactory_ = new FighterFactory(*scene_);
	initScene(fighter1, fighter2);
	initArena();
	initCamera();
	initColliderLayers();
	gameStateManager_ = new GameStateManager(fighters_[0], fighters_[1], debugDrawManager, *arena_);
}

FightingAppState::~FightingAppState()
{
	delete inputHandlerLeft_;
	delete inputHandlerRight_;
	delete fighterFactory_;
	delete arena_;
}

void FightingAppState::enterState()
{}

void FightingAppState::initScene(std::string fighterFilePath1, std::string fighterFilePath2)
{
	const glm::vec3 STARTING_POSITION_LEFT =  { 5.0f, 0.0f, 0.0f};
	const glm::vec3 STARTING_POSITION_RIGHT = { -5.0f, 0.0f, 0.0f};

	fighters_.resize(2);
	//fighters_[0] = fighterFactory_->createFighter(fighterFilePath1, *inputHandlerLeft_);
	fighters_[0] = fighterFactory_->createFighterNew(fighterFilePath1, *inputHandlerLeft_);
	fighters_[1] = fighterFactory_->createFighterNew(fighterFilePath2, *inputHandlerRight_);

	Transform& transform1 = fighters_[0]->getComponent<Transform>();
	Transform& transform2 = fighters_[1]->getComponent<Transform>();
	transform1.position_ = STARTING_POSITION_LEFT;
	transform2.position_ = STARTING_POSITION_RIGHT;

	Fighter* fighterLeft = Fighter::getFighterComp(fighters_[0]);
	fighterLeft->side_ = FighterSide::left;

	glm::vec3 pos1 = transform1.position_;
	glm::vec3 pos2 = transform2.position_;

	transform1.lookAt(pos2 - pos1);
	transform2.lookAt(pos1 - pos2);
	//generateArenaBackground();
}

void FightingAppState::initCamera()
{
	fightingGameCamera_ = scene_->createCameraEntity();
	fightingGameCamera_->addComponent<Behavior>(new FightingAppCameraBehavior(fightingGameCamera_, fighters_));
	scene_->setActiveCamera(fightingGameCamera_);

	flyCamera_ = scene_->createCameraEntity();
	flyCamera_->addComponent<Behavior>(new ControllableCameraBehavior(flyCamera_));
}

void FightingAppState::initColliderLayers()
{
	BoxCollisionManager* boxCollisionManager = BoxCollisionManager::getSingletonPtr();
	boxCollisionManager->addLayerRule(HitboxLayers::PUSH_BOX, HitboxLayers::PUSH_BOX);
	boxCollisionManager->addLayerRule(HitboxLayers::HURT_BOX, HitboxLayers::HIT_BOX);
}

void FightingAppState::checkInput()
{
	Input& input = Input::getSingleton();
	if (input.getButton("Fire3") > 0.0f)
	{
		scene_->setActiveCamera(flyCamera_);
	}
	else if (input.getButton("Fire2") > 0.0f)
	{
		scene_->setActiveCamera(fightingGameCamera_);
	}
}

void FightingAppState::initArena()
{
	arena_ = new Arena();
	arena_->pos = glm::vec3(0.0f);
	arena_->size = glm::vec3(21.0f, 1.0f, 21.0f);
	arena_->entity = scene_->addEntity("Arena");
	arena_->entity->addComponent<Transform>(arena_->pos);
	arena_->entity->addComponent<Behavior>(new ArenaFloorBehavior(arena_->entity, *arena_));
	//arena_->entity->addComponent<Collider>(arena_->entity, arena_->size, glm::vec3(0.0f, 0.0f, 0.0f), 0);

	auto createWallEntity = [&](ArenaWallBehavior::ArenaWallSide side)
	{
		Entity* entity = scene_->addEntity("ArenaWall");
		entity->addComponent<Transform>(0.0f, 0.0f, 0.0f);
		glm::vec3 size = glm::vec3(arena_->size.y, arena_->size.x, arena_->size.z);
		//Collider collider = Collider(entity, size, glm::vec3(0.0f), 0);
		//entity->addComponent<Collider>(collider);
		entity->addComponent<Behavior>(new ArenaWallBehavior(entity, *arena_, side));
		return entity;
	};

	arena_->walls.push_back(createWallEntity(ArenaWallBehavior::ArenaWallSide::LEFT));
	arena_->walls.push_back(createWallEntity(ArenaWallBehavior::ArenaWallSide::RIGHT));
}

void FightingAppState::generateArenaBackground()
{
	//const glm::vec3 BACKGROUND_POS = { 16.0f, 11.0f, -1.5f };//set position of background here
	//
	//const std::string arenaBackground = "./Models/WaverlyTownHall-02_1mil_meters_sf.obj";
	//const std::string arenaBackgroundTexturePath = "./Textures/WaverlyTownHall-02_1mil_meters_sf.jpg";

	//arena_.entity = scene_->addEntity("ArenaBackground");
	//const ModelReturnVals& backgroundModelData = resourceManager_->loadObjFile(arenaBackground);
	//const TextureReturnVals& textureReturnVals = resourceManager_->loadTextureFile(arenaBackgroundTexturePath);
	//arena_.entity->addComponent<Renderable>(backgroundModelData.vertices, backgroundModelData.indices, false, "ArenaBackground");
	//arena_.entity->addComponent<Textured>(textureReturnVals.pixels, textureReturnVals.textureWidth, textureReturnVals.textureHeight, textureReturnVals.textureChannels, "ArenaBackground");
	//Transform& arenaTransform = arena_.entity->addComponent<Transform>(BACKGROUND_POS.x, BACKGROUND_POS.y, BACKGROUND_POS.z);
}


AppState* FightingAppState::update(float deltaTime)
{
	gameStateManager_->update(deltaTime);
	checkInput();
	return nullptr;
}
