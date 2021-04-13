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
	
	std::string modelPath = "./Models/goblin.dae";
	std::string texturePath = "./Textures/viking_room.png";
	fighter_ = fighterFactory_->createFighter(modelPath, texturePath);
	//ModelReturnVals vals = resourceManager_->loadObjFile(modelPath);
	//TextureReturnVals texVals = resourceManager_->loadTextureFile(texturePath);


	//Entity& e = scene_->addEntity("Temp");
	//Transform& t = e.addComponent<Transform>( 1.0f,0.0f,0.0f );
	//t.drawDebugGui_ = true;
	//t.setScale(0.5f);
	//Renderable& r = e.addComponent<Renderable>(vals.vertices, vals.indices, true, "Viking_Room");
	//Textured& tex = e.addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Viking_Room");
	//
	//entities_.push_back(e);

	//std::string goblinPath = "./Models/goblin.dae";
	//std::string goblinTexturePath = "./Textures/missingTexture.jpg";
	//texVals = resourceManager_->loadTextureFile(goblinTexturePath);
	//Entity& goblin = scene_->addEntity("goblin");
	//AnimationReturnVals retVals = resourceManager_->loadAnimationFile(goblinPath);
	//Renderable& gRenderable = goblin.addComponent<Renderable>(retVals.vertices, retVals.indices, true, "goblin");
	//Transform& goblinT = goblin.addComponent<Transform>( 1.0f,0.0f,0.0f );
	//goblin.addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Viking_Room");
	//goblinT.setScale(0.001f);
	////goblinT.drawDebugGui_ = true;
	//goblin.addComponent<Animator>(retVals.animations, retVals.boneStructure).setAnimation(0);

	//entities_.push_back(goblin);


	//Entity& goblin2 = scene_->addEntity("goblin2");
	//Renderable& gRenderable2 = goblin2.addComponent<Renderable>(retVals.vertices, retVals.indices, true, "goblin2");
	//Transform& t2 = goblin2.addComponent<Transform>(5000.0f,0.0f,0.0f );
	//goblin2.addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Viking_Room");
	//t2.setScale(0.001f);
	////BoneStructure& gBones2 = goblin2.addComponent<BoneStructure>(retVals.boneMapping, retVals.boneInfo);
	//goblin2.addComponent<AnibaseCameramator>(retVals.animations, retVals.boneStructure).setAnimation(0);

	//entities_.push_back(goblin2);
	BaseCamera* cam = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });

	cameraController_ = new CameraController(*cam);

	scene_->addCamera(cam);

	debugManager_->drawGrid({ 255, 255, 255 }, true);
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
		setCursor(drawDebug);
		break;

	case 259: //backspace
		break;
	}

}

void Sandbox::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::KeyPressedEvent>(std::bind(&Sandbox::handleKeyButtonDown, this, std::placeholders::_1));
	d.dispatch<Events::KeyPressedEvent>(std::bind(&CameraController::handleKeyPressed, cameraController_, std::placeholders::_1));
	d.dispatch<Events::KeyReleasedEvent>(std::bind(&CameraController::handleKeyReleased, cameraController_, std::placeholders::_1));
	d.dispatch<Events::MouseMoveEvent>(std::bind(&CameraController::handleMouseMoved, cameraController_, std::placeholders::_1));

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
	fighter_->onUpdate(deltaTime);
	cameraController_->onUpdate(deltaTime);
}

Application* createApplication()
{
	return new Sandbox();
}
