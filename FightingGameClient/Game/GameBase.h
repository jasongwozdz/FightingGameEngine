#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"
#include "BaseCamera.h"
#include "CameraController.h"
#include "FightingGame/InputHandler.h"
#include "FightingGame/FighterFactory.h"
#include "FightingGame/GameStateManager.h"
#include "FightingGame/AppStates/AppState.h"

class CharacterSelectAppState;

class GameBase : public Application
{
public:
	GameBase() = default;
	virtual ~GameBase();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	void onEvent(Events::Event& e);
	void handleKeyButtonDown(Events::KeyPressedEvent& e);

	const std::string& FOLDER_LOCATION = "../FighterFiles/";//where .fganim files are saved by fighterCreatorTool

	AppState* currentAppState_;
	CharacterSelectAppState* characterSelectAppState_;

	BaseCamera* camera_;
	CameraController* cameraController_;
	InputHandler* inputHandler_;
	InputHandler* inputHandlerRight_;
	FighterFactory* fighterFactory_;
	GameStateManager* gameStateManager_;

	bool drawDebug_ = true;//should debug be drawn
	bool cursor_ = false;//is cursour disabled;
};

