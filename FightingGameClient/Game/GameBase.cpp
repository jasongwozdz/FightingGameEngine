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
	inputHandlerRight_ = new InputHandler(FightingGameInput::Side::rightSide);
	
	characterSelectAppState_ = new CharacterSelectAppState({ "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\Fighter1-NewAttack.fgAnim" }, inputHandler_, inputHandlerRight_, debugManager_, this);
	currentAppState_ = AppState::transitionAppState(characterSelectAppState_);
	addEventCallback(ENGINE_EVENT_CALLBACK(GameBase::onEvent));
}

GameBase::~GameBase()
{
	delete inputHandler_;
	delete inputHandlerRight_;
	delete characterSelectAppState_;
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
		//cameraController_->setCurrentCamera(scene_->getCurrentCamera());
		break;
	}
}

void GameBase::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::KeyPressedEvent>(std::bind(&GameBase::handleKeyButtonDown, this, std::placeholders::_1));

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
}

Application* createApplication()
{
	return new GameBase();
}
