#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"
#include "BaseCamera.h"
#include "CameraController.h"
#include "FightingGame/InputHandler.h"
#include "FightingGame/FighterFactory.h"
#include "FightingGame/FighterCamera.h"
#include "FightingGame/GameStateManager.h"


class GameBase : public Application
{
public:
	GameBase() = default;
	virtual ~GameBase();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	void initScene();
	void onEvent(Events::Event& e);
	void handleMouseClick(Events::MousePressedEvent& e);
	void handleKeyButtonDown(Events::KeyPressedEvent& e);
	void generateArenaBackground();

	const std::string& FOLDER_LOCATION = "../FighterFiles/";//where .fganim files are saved by fighterCreatorTool

	BaseCamera* camera_;
	CameraController* cameraController_;
	FighterCamera* fighterCamera_;
	InputHandler* inputHandler_;
	InputHandler* inputHandlerRight_;
	FighterFactory* fighterFactory_;
	GameStateManager* gameStateManager_;

	Arena arena_;
	Fighter* fighter_;
	Fighter* fighter2_;

	bool drawDebug_ = true;//should debug be drawn
	bool cursor_ = false;//is cursour disabled;
};

