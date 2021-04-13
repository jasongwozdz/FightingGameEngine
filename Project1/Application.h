#pragma once
#include <NewRenderer/VkRenderer.h>
#include <Scene/Scene.h>
#include "Engine/ResourceManager.h"
#include "Engine/EngineSettings.h"
#include "DebugDrawManager.h"

class Application{
public:
	Application();
	virtual ~Application();
	virtual void onUpdate(float timeDelta) = 0;
	virtual void onStartup() = 0;
	void addEventCallback(std::function<void(Events::Event&)>);
	void setCursor(bool showCursor);

	void run();
	// Singletons
	ResourceManager* resourceManager_;
	VkRenderer* renderer_;
	EngineSettings* engineSettings_;
	Window* window_;
	Scene* scene_;
	DebugDrawManager* debugManager_;

private:
	std::vector<std::function<void(Events::Event&)>> callbacks_;
	float endTime_;
	std::map<int, int> keyBinds_;
	bool drawUi_;
	void onEvent(Events::Event& e);
	void cleanup();
};

Application* createApplication(); //define in client and called in main

