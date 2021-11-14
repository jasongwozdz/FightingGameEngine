#pragma once
#include <string>

#include "AppState.h"
#include "Events.h"
#include "BaseCamera.h"
#include "../../CameraController.h"
#include "../InputHandler.h"
#include "../FighterFactory.h"
#include "../FighterCamera.h"
#include "../GameStateManager.h"

class FightingAppState :
	public AppState
{
public:
	FightingAppState(std::string fighter1, std::string fighter2, DebugDrawManager* debugDrawManager, InputHandler* inputLeft, InputHandler* inputRight);
	~FightingAppState();
	virtual void enterState();
	virtual AppState* update(float deltaTime);
	void initScene(std::string fighterFilePath1, std::string fighterFilePath2);
	void generateArenaBackground();
	void handleKeyButtonDown(Events::KeyPressedEvent& e);
private:
	BaseCamera* camera_;
	CameraController* cameraController_;
	FighterCamera* fighterCamera_;
	InputHandler* inputHandlerLeft_;
	InputHandler* inputHandlerRight_;
	FighterFactory* fighterFactory_;
	GameStateManager* gameStateManager_;
	ResourceManager* resourceManager_;
	DebugDrawManager* debugDrawManager_;

	Arena arena_;
	Fighter* fighter_;
	Fighter* fighter2_;

	bool drawDebug_ = false;//should debug be drawn
	bool cursor_ = false;//is cursour disabled;

	Scene* scene_;

};

