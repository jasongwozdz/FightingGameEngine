#pragma once
#include "Engine/Renderer/VkRenderer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ResourceManager.h"
#include "Engine/EngineSettings.h"
#include "Engine/DebugDrawManager.h"
#include "Engine/BoxCollisionManager.h"
#include "GLFW/glfw3.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#define ENGINE_EVENT_CALLBACK(Function) std::bind(&Function, this, std::placeholders::_1)

class Console;

class ENGINE_API Application{
public:
	Application();
	virtual ~Application();
	virtual void onUpdate(float timeDelta) = 0;
	virtual void onStartup() = 0;
	void addEventCallback(std::function<void(Events::Event&)>);
	void setCursor(bool showCursor);

	void run();
public:
	// Singletons
	ResourceManager* resourceManager_;
	VkRenderer* renderer_;
	EngineSettings* engineSettings_;
	Window* window_;
	Scene* scene_;
	DebugDrawManager* debugManager_;
	class Input* input_;
	Console* console_;

	static int FrameCount;

private:
	std::vector<std::function<void(Events::Event&)>> callbacks_;
	std::map<int, int> keyBinds_;
	bool drawUi_;
	void onEvent(Events::Event& e);
	void cleanup();
};

Application* createApplication(); //define in client and called in main

