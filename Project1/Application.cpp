#include <ctime>
#include "Application.h"
#include "Engine/GameObject.h"

Application::Application()
{
	gObjectManager = new GameObjectManager();
	resourceManager = new ResourceManager();
	engineSettings = new EngineSettings();
	engineSettings->init();
	pipelineManager = new PipelineManager();
	window = new Window();
	window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	renderer = new Renderer();
	debugDrawManager = new DebugDrawManager();
	pipelineManager->init();
	renderer->init(window);
	debugDrawManager->init();
}

Application::~Application()
{
	cleanup();
}

void Application::addEventCallback(std::function<void(Events::Event&)> fn)
{
	callbacks.push_back(fn);
}

void Application::onEvent(Events::Event& e)
{
	Events::EventDispatcher dispatcher(e);
	//callbacks defined in client
	for (auto fn : callbacks)
	{
		fn(e);
	}
}

void Application::cleanup()
{
	delete gObjectManager;
	delete resourceManager;
	delete engineSettings;
	delete pipelineManager;
	delete window;
	delete ui;
	delete renderer;
	delete debugDrawManager;
}

void Application::run()
{
	float endTime = 0;
	float deltaTime = 0;
	onStartup();
	while (!glfwWindowShouldClose(window->getGLFWWindow()))
	{
		float start = endTime;
		onUpdate(deltaTime);
		gObjectManager->update(deltaTime);
		window->onUpdate();
		renderer->drawFrame();
		endTime = std::clock();
		deltaTime = endTime - start;
	}
	vkDeviceWaitIdle(renderer->getLogicalDevice());
}
