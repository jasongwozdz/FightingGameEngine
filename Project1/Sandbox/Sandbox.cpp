#include <iostream>
#include "Sandbox.h"
#include "../EntryPoint.h"
#include <glm/gtx/matrix_decompose.hpp>

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
	delete camera_;
	delete cameraController_;
	delete fighterCamera_;
	delete inputHandler_;
	delete fighterFactory_;
	delete fighter_;
	delete fighter2_;
}


void Sandbox::initScene()
{
	std::string modelPath = "./Models/goblin.dae";
	std::string texturePath = "./Textures/viking_room.png";
	fighter_ = fighterFactory_->createFighter(modelPath, texturePath, *inputHandler_);
	fighter_->setPosition({0.0f, 5.0f, 0.0f});
	fighter_->controllable_ = true;
	fighter2_ = fighterFactory_->createFighter(modelPath, texturePath, *inputHandlerRight_);
	fighter2_->controllable_ = true;
	fighter2_->setPosition({0.0f, -5.0f, 0.0f});
	fighter2_->flipSide();

	joint_ = scene_->addEntity("Joint");
	Transform& t = joint_->addComponent<Transform>(glm::vec3(0,0,0));
	//t.parent_ = fighter_->entity_;

	Hitbox& h = fighter_->entity_->getComponent<Hitbox>();
	h.hitboxEnt_ = debugManager_->drawRect( h.pos_, { 0, 10, 0 }, 0, true, -h.width_/2, h.width_/2, -h.height_/2,	h.height_/2, fighter_->entity_ );

	h = fighter2_->entity_->getComponent<Hitbox>();
	h.hitboxEnt_ = debugManager_->drawRect( h.pos_, { 0, 10, 0 }, 0, true, -h.width_/2, h.width_/2, -h.height_/2,	h.height_/2, fighter2_->entity_ );

	camera_ = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	BaseCamera* debugCamera = new BaseCamera({ 10.0f, 3.0f, 1.0f }, { 1.0f, -3.0f, -1.0f }, { 0.0f, 0.0f, 1.0f });
	camera_->drawDebug_ = true;

	cameraController_ = new CameraController(*debugCamera);
	fighterCamera_ = new FighterCamera(camera_, fighter_, fighter2_);

	scene_->addCamera(camera_);
	scene_->addCamera(debugCamera);

#define GRID_WIDTH 21
#define GRID_DEPTH 21
	arena_ = { GRID_WIDTH, GRID_DEPTH, {0.0, 0.0, -1.75f} };

	debugManager_->drawGrid(arena_.pos, GRID_WIDTH, GRID_DEPTH, { 255, 255, 255 }, true);
	//debugManager_->addPoint({ 0, 0, 0 }, { 1, 1, 1 }, 0, true, fighter_->entity_);


	debugManager_->addPoint({ 0, arena_.pos.y + arena_.width/2, 0 }, { 0.0f, 10.0f, 0.0f }, 0, true);
	debugManager_->addPoint({ 0, arena_.pos.y - arena_.width/2, 0 }, { 0.0f, 10.0f, 0.0f }, 0, true);
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

	d.dispatch<Events::KeyPressedEvent>(std::bind(&InputHandler::handleInputPressed, inputHandlerRight_, std::placeholders::_1));
	d.dispatch<Events::KeyReleasedEvent>(std::bind(&InputHandler::handleInputReleased, inputHandlerRight_, std::placeholders::_1));
}

void Sandbox::onStartup()
{
	inputHandler_ = new InputHandler();
	inputHandlerRight_ = new InputHandler();
	inputHandlerRight_->side_ = Input::Side::rightSide;

	fighterFactory_ = new FighterFactory(*scene_, *inputHandler_);
	initScene();
	gameStateManager_ = new GameStateManager(fighter_, fighter2_, *debugManager_, GRID_WIDTH, GRID_DEPTH);

	addEventCallback(std::bind(&Sandbox::onEvent, this, std::placeholders::_1));
}

glm::mat4 getGlobalTransformOfBone(const BoneStructure& boneStruct, int jointIndex)
{
	int parent = 0;
	glm::mat4 finalTransform(1.0f);
	//while (parent != -1)
	//{

	//	parent = boneStruct.boneInfo_[jointIndex].parent_
	//}
	return finalTransform;
}

void setJointPos(Entity& fighter, Entity& joint, Entity& point, int jointIndex)
{
	Animator& animator = fighter.getComponent<Animator>();
	if (animator.globalTransforms.size() > jointIndex)
	{
		Transform& fighterTransform = fighter.getComponent<Transform>();
		Renderable& mesh = fighter.getComponent<Renderable>();
		Transform& jointTransform = joint.getComponent<Transform>();
		glm::mat4 globalTransform = animator.globalTransforms[jointIndex];
		//globalTransform = glm::scale(globalTransform, { 0.001f, 0.001f, 0.001f });
		point.getComponent<Transform>().finalTransform_ = globalTransform;
		point.getComponent<Transform>().calculateTransform_ = false;

		//fighterTransform.setScale(0.001f);
		//glm::vec3 scale;
		//glm::quat rotation;
		//glm::vec3 translation;
		//glm::vec3 skew;
		//glm::vec4 perspective;
		//glm::decompose(finalTransform, scale, rotation, translation, skew, perspective);

		//jointTransform.pos_ = -translation;
		//jointTransform.rot_ = -rotation;
	}
}

void Sandbox::onUpdate(float deltaTime)
{
	gameStateManager_->update(0);
	//setJointPos(*fighter_->entity_, *joint_, *hurtboxDebug_, 3);
	//updateAttack(*fighter_, attacks_[0], *hurtboxDebug_);

	if (fighter_)
		fighter_->onUpdate(deltaTime);
	if (fighter2_)
		fighter2_->onUpdate(deltaTime);
	if (fighterCamera_ && !drawDebug)
		fighterCamera_->onUpdate(deltaTime);
	if (cameraController_)
		cameraController_->onUpdate(deltaTime);
	if (camera_)
		camera_->update(deltaTime);
}

Application* createApplication()
{
	return new Sandbox();
}
