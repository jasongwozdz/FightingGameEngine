#include <iostream>
#include "Sandbox.h"
#include "../EntryPoint.h"
#include "DebugCamera.h"

/*
Severity	Code	Description	File	Line	Project	Suppression State
Error	LNK1168	cannot open C:\Users\jsngw\source\repos\AnimationTest\x64\Debug\VulkanAnimation.exe for writing	C:\Users\jsngw\source\repos\AnimationTest\Project1\LINK	1	VulkanAnimation	

DO THIS TO FIX
taskkill /F /IM VulkanAnimation.exe

*/
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

Sandbox::~Sandbox()
{

}

void Sandbox::initScene()
{
	debugManager_->drawGrid({ 255, 255, 255 }, true);
	std::string modelPath = "./Models/goblin.dae";
	std::string texturePath = "./Textures/viking_room.png";
	fighter_ = fighterFactory_->createFighter(modelPath, texturePath);
	fighter_->setPosition({0.0f, 10.0f, 0.0f});
	fighter_->controllable_ = true;
	fighter2_ = fighterFactory_->createFighter(modelPath, texturePath);
	fighter2_->controllable_ = true;
	fighter2_->setPosition({0.0f, -10.0f, 0.0f});
	fighter2_->flipSide();

	//modelPath = "./Models/Viking_room.obj";
	//ModelReturnVals vals = resourceManager_->loadObjFile(modelPath);
	//TextureReturnVals texVals = resourceManager_->loadTextureFile(texturePath);
	//Entity& e = scene_->addEntity("Temp");
	//Transform& t = e.addComponent<Transform>(1.0f, 0.0f, 0.0f);
	//t.flipScale();
	//Renderable& r = e.addComponent<Renderable>(vals.vertices, vals.indices, true, "Viking_Room");
	//Textured& tex = e.addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Viking_Room");

	camera_ = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	BaseCamera* debugCamera = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	camera_->drawDebug_ = true;

	cameraController_ = new CameraController(*debugCamera);

	fighterCamera_ = new FighterCamera(camera_, fighter_, fighter2_);

	scene_->addCamera(camera_);
	scene_->addCamera(debugCamera);

	debugManager_->addPoint({ 0, 0, 0 }, { 1, 1, 1 }, 0, true);
}

void Sandbox::handleMouseClick(Events::MousePressedEvent& e)
{
}

void Sandbox::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	switch (e.KeyCode)
	{
	case 256: //esc
		drawDebug = !drawDebug;
		if (drawDebug)
		{
			scene_->setCamera(1);
			cameraController_->controllable_ = !cursor_;
			fighter_->controllable_ = false;
			fighter2_->controllable_ = false;
		}
		else
		{
			scene_->setCamera(0);
			cameraController_->controllable_ = false;
			fighter_->controllable_ = true;
			fighter2_->controllable_ = true;
		}
		break;

	case 259: //backspace
		cursor_ = !cursor_;
		setCursor(cursor_);
		cameraController_->controllable_ = !cursor_;
		break;
	}

}

void Sandbox::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::KeyPressedEvent>(std::bind(&Sandbox::handleKeyButtonDown, this, std::placeholders::_1));
	if (cameraController_)
	{
		d.dispatch<Events::KeyPressedEvent>(std::bind(&CameraController::handleKeyPressed, cameraController_, std::placeholders::_1));
		d.dispatch<Events::KeyReleasedEvent>(std::bind(&CameraController::handleKeyReleased, cameraController_, std::placeholders::_1));
		d.dispatch<Events::MouseMoveEvent>(std::bind(&CameraController::handleMouseMoved, cameraController_, std::placeholders::_1));
	}

	d.dispatch<Events::KeyPressedEvent>(std::bind(&InputHandler::handleInputPressed, inputHandler_, std::placeholders::_1));
	d.dispatch<Events::KeyReleasedEvent>(std::bind(&InputHandler::handleInputReleased, inputHandler_, std::placeholders::_1));
}

void Sandbox::onStartup()
{
	inputHandler_ = new InputHandler();
	fighterFactory_ = new FighterFactory(*scene_, *inputHandler_);
	initScene();
	addEventCallback(std::bind(&Sandbox::onEvent, this, std::placeholders::_1));
}

void Sandbox::onUpdate(float deltaTime)
{
	if(fighter_)
		fighter_->onUpdate(deltaTime);
	if(fighter2_)
		fighter2_->onUpdate(deltaTime);
	if (fighterCamera_ && !drawDebug)
		fighterCamera_->onUpdate(deltaTime);
	if(cameraController_)
		cameraController_->onUpdate(deltaTime);
	if (camera_)
		camera_->update(deltaTime);
}

Application* createApplication()
{
	return new Sandbox();
}
