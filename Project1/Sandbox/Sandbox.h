#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"
#include "CameraController.h"
#include "FightingGame/InputHandler.h"
#include "FightingGame/FighterFactory.h"
#include "FightingGame/FighterCamera.h"

class Sandbox : public Application
{
public:
	virtual ~Sandbox();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	BaseCamera* camera_;
	CameraController* cameraController_;
	FighterCamera* fighterCamera_;
	InputHandler* inputHandler_;
	FighterFactory* fighterFactory_;
	Fighter* fighter_;
	Fighter* fighter2_;
	bool cursor_ = false;

	bool drawDebug;
	std::vector<Entity> entities_;
	

	void initScene();
	void onEvent(Events::Event& e);
	void handleMouseClick(Events::MousePressedEvent& e);
	void handleKeyButtonDown(Events::KeyPressedEvent& e);

};

