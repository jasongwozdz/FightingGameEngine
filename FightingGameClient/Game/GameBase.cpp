#include <iostream>
#include "GameBase.h"
#include "../EntryPoint.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "FightingGame/FighterFileImporter.h"

/*
Severity	Code	Description	File	Line	Project	Suppression State
Error	LNK1168	cannot open C:\Users\jsngw\source\repos\AnimationTest\x64\Debug\VulkanAnimation.exe for writing	C:\Users\jsngw\source\repos\AnimationTest\Project1\LINK	1	VulkanAnimation	

DO THIS TO FIX
taskkill /F /IM VulkanAnimation.exe

*/
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

GameBase::~GameBase()
{
	delete camera_;
	delete cameraController_;
	delete fighterCamera_;
	delete inputHandler_;
	delete fighterFactory_;
	delete fighter_;
	delete fighter2_;
}

void GameBase::initScene()
{
	const glm::vec3 STARTING_POSITION_RIGHT = { 0.0f, 5.0f, 1.0f };
	const glm::vec3 STARTING_POSITION_LEFT =  {0.0f, -5.0f, 1.0f};

	const std::string fighterFilePath = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\Attack.fgAnim";
	fighter_ = fighterFactory_->createFighter(fighterFilePath, *inputHandler_);
	fighter_->setPosition(STARTING_POSITION_RIGHT);
	fighter_->controllable_ = true;
	fighter2_ = fighterFactory_->createFighter(fighterFilePath, *inputHandlerRight_);
	fighter2_->controllable_ = true;
	fighter2_->setPosition(STARTING_POSITION_LEFT);

	camera_ = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	fighterCamera_ = new FighterCamera(camera_, fighter_, fighter2_);

	BaseCamera* debugCamera = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	cameraController_ = new CameraController(*debugCamera);

	scene_->addCamera(camera_);
	scene_->addCamera(debugCamera);

	const glm::vec3 ARENA_STARTING_POINT =  {0.0, 0.0, -1.75f};
	const float ARENA_WIDTH = 21;
	const float ARENA_DEPTH = 21;
	arena_ = { ARENA_WIDTH, ARENA_DEPTH, ARENA_STARTING_POINT };

	//generateArenaBackground();
}

void GameBase::generateArenaBackground()
{
	const glm::vec3 ARENA_STARTING_POINT =  {0.0, 0.0, -1.75f};
	const float ARENA_WIDTH = 21;
	const float ARENA_DEPTH = 21;
	
	const std::string arenaBackground = "./Models/viking_room.obj";
	const std::string arenaBackgroundTexturePath = "./Textures/viking_room.png";

	arena_.backgroundEntity = scene_->addEntity("ArenaBackground");
	const ModelReturnVals& backgroundModelData = resourceManager_->loadObjFile(arenaBackground);
	const TextureReturnVals& textureReturnVals = resourceManager_->loadTextureFile(arenaBackgroundTexturePath);
	arena_.backgroundEntity->addComponent<Renderable>(backgroundModelData.vertices, backgroundModelData.indices, false, "ArenaBackground");
	arena_.backgroundEntity->addComponent<Textured>(textureReturnVals.pixels, textureReturnVals.textureWidth, textureReturnVals.textureHeight, textureReturnVals.textureChannels, "ArenaBackground");
	Transform& arenaTransform = arena_.backgroundEntity->addComponent<Transform>(-10.0f, 0.0f, -1.5f);
	arenaTransform.scale_ = { 15.0f, 15.0f, 15.0f };
}

void GameBase::handleMouseClick(Events::MousePressedEvent& e)
{
}

void GameBase::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	switch (e.KeyCode)
	{
	case 256: //esc
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

	case 259: //backspace
		cursor_ = !cursor_;
		setCursor(cursor_);
		cameraController_->controllable_ = !cursor_;
		break;

	case GLFW_KEY_F:
		gameStateManager_->debug_ = !gameStateManager_->debug_;
	}
}

void GameBase::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::KeyPressedEvent>(std::bind(&GameBase::handleKeyButtonDown, this, std::placeholders::_1));
	if (cameraController_)
	{
		d.dispatch<Events::KeyPressedEvent>(std::bind(&CameraController::handleKeyPressed, cameraController_, std::placeholders::_1));
		d.dispatch<Events::KeyReleasedEvent>(std::bind(&CameraController::handleKeyReleased, cameraController_, std::placeholders::_1));
		d.dispatch<Events::MouseMoveEvent>(std::bind(&CameraController::handleMouseMoved, cameraController_, std::placeholders::_1));
	}

	d.dispatch<Events::KeyPressedEvent>(std::bind(&InputHandler::handleInputPressed, inputHandler_, std::placeholders::_1));
	d.dispatch<Events::KeyReleasedEvent>(std::bind(&InputHandler::handleInputReleased, inputHandler_, std::placeholders::_1));

	d.dispatch<Events::KeyPressedEvent>(std::bind(&InputHandler::handleInputPressed, inputHandlerRight_, std::placeholders::_1));
	d.dispatch<Events::KeyReleasedEvent>(std::bind(&InputHandler::handleInputReleased, inputHandlerRight_, std::placeholders::_1));
}

void GameBase::onStartup()
{
	inputHandler_ = new InputHandler();
	inputHandlerRight_ = new InputHandler();
	inputHandlerRight_->side_ = Input::Side::rightSide;

	fighterFactory_ = new FighterFactory(*scene_);
	initScene();
	gameStateManager_ = new GameStateManager(fighter_, fighter2_, debugManager_, arena_);

	addEventCallback(std::bind(&GameBase::onEvent, this, std::placeholders::_1));
}

glm::mat4 getGlobalTransformOfBone(const BoneStructure& boneStruct, int jointIndex)
{
	int parent = 0;
	glm::mat4 finalTransform(1.0f);
	return finalTransform;
}

void GameBase::onUpdate(float deltaTime)
{
	Transform& t = fighter_->entity_->getComponent<Transform>();
	Transform& t2 = fighter2_->entity_->getComponent<Transform>();
	
	if (fighter_)
		fighter_->onUpdate(deltaTime);
	if (fighter2_)
		fighter2_->onUpdate(deltaTime);
	if (fighterCamera_ && !drawDebug_)
		fighterCamera_->onUpdate(deltaTime);
	if (cameraController_)
		cameraController_->onUpdate(deltaTime);
	if (camera_)
		camera_->update(deltaTime);

	gameStateManager_->update(0);
}

Application* createApplication()
{
	return new GameBase();
}
