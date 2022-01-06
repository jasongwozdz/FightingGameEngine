#include "FightingAppState.h"
#include "ResourceManager.h"
#include "Scene/Components/Camera.h"
#include "../ControllableCameraBehavior.h"
#include "Input.h"
#include "../Arena.h"
#include "../ArenaFloorBehavior.h"

FightingAppState::FightingAppState(std::string fighter1, std::string fighter2, DebugDrawManager* debugDrawManager, InputHandler* inputLeft, InputHandler* inputRight) : 
	scene_(Scene::getSingletonPtr()),
	inputHandlerLeft_(inputLeft),
	inputHandlerRight_(inputRight),
	debugDrawManager_(debugDrawManager)
{
	inputHandlerLeft_->clearInputQueue();
	inputHandlerRight_->clearInputQueue();
	fighterFactory_ = new FighterFactory(*scene_);
	initScene(fighter1, fighter2);
	gameStateManager_ = new GameStateManager(fighters_[0], fighters_[1], debugDrawManager/*, arena_*/);
	initalizeCamera();
}

FightingAppState::~FightingAppState()
{
	delete inputHandlerLeft_;
	delete inputHandlerRight_;
	delete fighterFactory_;
}

void FightingAppState::enterState()
{}

void FightingAppState::initScene(std::string fighterFilePath1, std::string fighterFilePath2)
{
	const glm::vec3 STARTING_POSITION_LEFT =  { 5.0f, 0.0f, 0.0f};
	const glm::vec3 STARTING_POSITION_RIGHT = { -5.0f, 0.0f, 0.0f};

	fighters_.resize(2);

	fighters_[0] = fighterFactory_->createFighter(fighterFilePath1, *inputHandlerLeft_);
	fighters_[1] = fighterFactory_->createFighter(fighterFilePath2, *inputHandlerRight_);

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
	initArena();
	//generateArenaBackground();
}


void FightingAppState::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	switch (e.KeyCode)
	{
	case GLFW_KEY_ESCAPE: //esc
	{
	/*	drawDebug_ = !drawDebug_;
		if (drawDebug_)
		{
			fighter_->controllable_ = false;
		}
		else
		{
			fighter_->controllable_ = true;
		}
		break;*/
	}
	case GLFW_KEY_F:
	{
		gameStateManager_->debug_ = !gameStateManager_->debug_;
		break;
	}
	}
}

void FightingAppState::initalizeCamera()
{
	fightingGameCamera_ = scene_->createCameraEntity();
	fightingGameCamera_->addComponent<Behavior>(new FightingAppCameraBehavior(fightingGameCamera_, fighters_));
	scene_->setActiveCamera(fightingGameCamera_);

	flyCamera_ = scene_->createCameraEntity();
	flyCamera_->addComponent<Behavior>(new ControllableCameraBehavior(flyCamera_));
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
	Arena arena = { ARENA_WIDTH, ARENA_DEPTH, ARENA_LENGTH, ARENA_STARTING_POINT };
	arena.entity = scene_->addEntity("Arena");
	arena.entity->addComponent<Transform>(arena.pos);
	arena.entity->addComponent<Behavior>(new ArenaFloorBehavior(arena.entity, arena));
	arena.entity->addComponent<Collider>(arena.entity, ARENA_LENGTH, arena.depth, arena.depth, glm::vec3(0.0f, 0.0f, 0.0f), 0);
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
	//Transform& t = fighter_->entity_->getComponent<Transform>();
	//Transform& t2 = fighter2_->entity_->getComponent<Transform>();
	
	//if (fighter_)
	//	fighter_->onUpdate(deltaTime, debugDrawManager_);
	//if (fighter2_)
	//	fighter2_->onUpdate(deltaTime, debugDrawManager_);
	//if (fighterCamera_ && !drawDebug_)
	//	fighterCamera_->onUpdate(deltaTime);
	//if (cameraController_)
	//	cameraController_->onUpdate(deltaTime);
	//if (camera_)
	//	camera_->update(deltaTime);

	gameStateManager_->update(deltaTime);

	//if (fighter_)
	//	fighter_->updateTransform();
	//if (fighter2_)
	//	fighter2_->updateTransform();

	checkInput();
	return nullptr;
}
