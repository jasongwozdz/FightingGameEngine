#include <ctime>
#include "Application.h"

Application::Application()
{
	resourceManager = new ResourceManager();
	engineSettings = new EngineSettings();
	engineSettings->init();
	window = new Window();
	window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	window->setCursor(true);
	renderer = new VkRenderer();
	renderer->init(*window);
	scene_ = new Scene();
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
	delete resourceManager;
	delete engineSettings;
	delete window;
	delete renderer;
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
		window->onUpdate();
		renderer->prepareFrame();
		scene_->update(deltaTime);
		renderer->draw();
		endTime = std::clock();
		deltaTime = endTime - start;
	}
	vkDeviceWaitIdle(renderer->logicalDevice_);
}
