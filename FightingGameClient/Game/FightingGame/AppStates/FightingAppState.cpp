#include "FightingAppState.h"
#include "ResourceManager.h"

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
	gameStateManager_ = new GameStateManager(fighter_, fighter2_, debugDrawManager, arena_);
	//scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/skybox");
}

FightingAppState::~FightingAppState()
{
	delete camera_;
	delete cameraController_;
	delete fighterCamera_;
	delete inputHandlerLeft_;
	delete inputHandlerRight_;
	delete fighterFactory_;
	delete fighter_;
	delete fighter2_;
}

void FightingAppState::enterState()
{}

void FightingAppState::initScene(std::string fighterFilePath1, std::string fighterFilePath2)
{
	const glm::vec3 STARTING_POSITION_RIGHT = { 0.0f, 5.0f, -1.75f };
	const glm::vec3 STARTING_POSITION_LEFT =  {0.0f, -5.0f, -1.75f};

	fighter_ = fighterFactory_->createFighter(fighterFilePath1, *inputHandlerLeft_);
	fighter_->setPosition(STARTING_POSITION_RIGHT);
	fighter_->controllable_ = true;
	fighter2_ = fighterFactory_->createFighter(fighterFilePath2, *inputHandlerRight_);
	fighter2_->controllable_ = true;
	fighter2_->setPosition(STARTING_POSITION_LEFT);

	camera_ = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	fighterCamera_ = new FighterCamera(camera_, fighter_, fighter2_);

	BaseCamera* debugCamera = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	cameraController_ = new CameraController(debugCamera);

	scene_->addCamera(camera_);
	scene_->addCamera(debugCamera);
	scene_->setCamera(camera_->cameraIndex_);


	const glm::vec3 ARENA_STARTING_POINT =  {0.0, 0.0, -1.75f};
	const float ARENA_WIDTH = 21;
	const float ARENA_DEPTH = 21;
	arena_ = { ARENA_WIDTH, ARENA_DEPTH, ARENA_STARTING_POINT };

	//generateArenaBackground();
}

void FightingAppState::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	switch (e.KeyCode)
	{
	case 256: //esc
	{
		drawDebug_ = !drawDebug_;
		if (drawDebug_)
		{
			scene_->setCamera(1);
			cameraController_->controllable_ = !cursor_;
			fighter_->controllable_ = false;
		}
		else
		{
			scene_->setCamera(0);
			cameraController_->controllable_ = false;
			fighter_->controllable_ = true;
		}
		break;
	}
	case GLFW_KEY_F:
	{
		gameStateManager_->debug_ = !gameStateManager_->debug_;
		break;
	}
	}
}

void FightingAppState::generateArenaBackground()
{
	const glm::vec3 BACKGROUND_POS = { 16.0f, 11.0f, -1.5f };//set position of background here
	
	const std::string arenaBackground = "./Models/WaverlyTownHall-02_1mil_meters_sf.obj";
	const std::string arenaBackgroundTexturePath = "./Textures/WaverlyTownHall-02_1mil_meters_sf.jpg";

	arena_.backgroundEntity = scene_->addEntity("ArenaBackground");
	const ModelReturnVals& backgroundModelData = resourceManager_->loadObjFile(arenaBackground);
	const TextureReturnVals& textureReturnVals = resourceManager_->loadTextureFile(arenaBackgroundTexturePath);
	arena_.backgroundEntity->addComponent<Renderable>(backgroundModelData.vertices, backgroundModelData.indices, false, "ArenaBackground");
	arena_.backgroundEntity->addComponent<Textured>(textureReturnVals.pixels, textureReturnVals.textureWidth, textureReturnVals.textureHeight, textureReturnVals.textureChannels, "ArenaBackground");
	Transform& arenaTransform = arena_.backgroundEntity->addComponent<Transform>(BACKGROUND_POS.x, BACKGROUND_POS.y, BACKGROUND_POS.z);
}


AppState* FightingAppState::update(float deltaTime)
{
	Transform& t = fighter_->entity_->getComponent<Transform>();
	Transform& t2 = fighter2_->entity_->getComponent<Transform>();
	
	if (fighter_)
		fighter_->onUpdate(deltaTime, debugDrawManager_);
	if (fighter2_)
		fighter2_->onUpdate(deltaTime, debugDrawManager_);
	if (fighterCamera_ && !drawDebug_)
		fighterCamera_->onUpdate(deltaTime);
	if (cameraController_)
		cameraController_->onUpdate(deltaTime);
	if (camera_)
		camera_->update(deltaTime);

	gameStateManager_->update(deltaTime);

	if (fighter_)
		fighter_->updateTransform();
	if (fighter2_)
		fighter2_->updateTransform();
	return nullptr;
}

