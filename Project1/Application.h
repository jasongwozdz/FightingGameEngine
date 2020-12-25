#pragma once
#include "Engine/Renderer/Renderer.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/DebugDrawManager.h"
#include "Engine/EngineSettings.h"

class Application
{
public:
	Application();
	virtual ~Application();
	virtual void onUpdate(float timeDelta) = 0;
	virtual void onStartup() = 0;
	void addEventCallback(std::function<void(Events::Event&)>);

	void run();
	// Singletons
	ResourceManager* resourceManager;
	Renderer* renderer;
	GameObjectManager* gObjectManager;
	DebugDrawManager* debugDrawManager;
	PipelineManager* pipelineManager;
	EngineSettings* engineSettings;
	UserInterface* ui;
	Window* window;
private:
	std::vector<std::function<void(Events::Event&)>> callbacks;
	float endTime;
	std::map<int, int> keyBinds;
	bool drawUi;

	void onEvent(Events::Event& e);
	void cleanup();
};

Application* createApplication(); //define in client and called in main

