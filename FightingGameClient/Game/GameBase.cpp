#include <iostream>
#include "GameBase.h"
#include "../EntryPoint.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "FightingGame/FighterFileImporter.h"
#include "FightingGame/AppStates/CharacterSelectAppState.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void GameBase::onStartup()
{

	inputHandler_ = new InputHandler();
	inputHandlerRight_ = new InputHandler(Input::Side::rightSide);
	cameraController_ = new CameraController();
	
	characterSelectAppState_ = new CharacterSelectAppState({ "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\Fighter1.fgAnim", "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\fighter2.fgAnim"}, inputHandler_, inputHandlerRight_, debugManager_);
	currentAppState_ = AppState::transitionAppState(characterSelectAppState_);
	addEventCallback(std::bind(&GameBase::onEvent, this, std::placeholders::_1));
}

GameBase::~GameBase()
{
	delete inputHandler_;
	delete inputHandlerRight_;
	delete characterSelectAppState_;
	delete cameraController_;
}


void GameBase::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	switch (e.KeyCode)
	{
	case 259: //backspace
		cursor_ = !cursor_;
		setCursor(cursor_);
		//cameraController_->controllable_ = !cursor_;
		break;
	case 256:
		cameraController_->setCurrentCamera(scene_->getCurrentCamera());
		break;
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

void GameBase::onUpdate(float deltaTime)
{
	AppState* newAppState;
	if ((newAppState = currentAppState_->update(deltaTime)) != nullptr)
	{
		currentAppState_ = AppState::transitionAppState(newAppState);
	}
	if (cameraController_)
	{
		cameraController_->onUpdate(deltaTime);
	}
	

	//Transform& t = fighter_->entity_->getComponent<Transform>();
	//Transform& t2 = fighter2_->entity_->getComponent<Transform>();
	//
	//if (fighter_)
	//	fighter_->onUpdate(deltaTime);
	//if (fighter2_)
	//	fighter2_->onUpdate(deltaTime);
	//if (fighterCamera_ && !drawDebug_)
	//	fighterCamera_->onUpdate(deltaTime);
	//if (cameraController_)
	//	cameraController_->onUpdate(deltaTime);
	//if (camera_)
	//	camera_->update(deltaTime);

	//gameStateManager_->update(deltaTime);

	//if (fighter_)
	//	fighter_->updateTransform();
	//if (fighter2_)
	//	fighter2_->updateTransform();
}

Application* createApplication()
{
	return new GameBase();
}
