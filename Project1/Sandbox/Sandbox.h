#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"
#include "CameraController.h"
#include "FightingGame/InputHandler.h"
#include "FightingGame/FighterFactory.h"
#include "FightingGame/FighterCamera.h"
#include "FightingGame/GameStateManager.h"


class Sandbox : public Application
{
public:
	Sandbox() = default;
	virtual ~Sandbox();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	Arena arena_;
	BaseCamera* camera_;
	CameraController* cameraController_;
	FighterCamera* fighterCamera_;
	InputHandler* inputHandler_;
	InputHandler* inputHandlerRight_;
	FighterFactory* fighterFactory_;

	GameStateManager* gameStateManager_;

	Fighter* fighter_;
	Fighter* fighter2_;
	Entity* joint_;
	bool cursor_ = false;

	Entity* hurtboxDebug_;

	bool drawDebug;
	std::vector<Entity> entities_;
	
	std::vector<Attack> attacks_;

	void initScene();
	void onEvent(Events::Event& e);
	void handleMouseClick(Events::MousePressedEvent& e);
	void handleKeyButtonDown(Events::KeyPressedEvent& e);

};

