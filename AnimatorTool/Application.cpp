#include <ctime>
#include "Application.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

Application::Application()
{
	resourceManager_ = new ResourceManager();
	engineSettings_ = new EngineSettings();
	engineSettings_->init();
	window_ = new Window();
	window_->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	renderer_ = new VkRenderer(*window_);
	renderer_->init();
	scene_ = new Scene();
	debugManager_ = renderer_->debugDrawManager_;
}

Application::~Application()
{
	cleanup();
	_CrtDumpMemoryLeaks();
}

void Application::addEventCallback(std::function<void(Events::Event&)> fn)
{
	callbacks_.push_back(fn);
}

void Application::onEvent(Events::Event& e)
{
	Events::EventDispatcher dispatcher(e);
	dispatcher.dispatch<Events::FrameBufferResizedEvent>(std::bind(&VkRenderer::frameBufferResizeCallback, renderer_, std::placeholders::_1));
	//callbacks defined in client
	for (auto fn : callbacks_)
	{
		fn(e);
	}
}

void Application::setCursor(bool showCursor)
{
	window_->setCursor(showCursor);
}

void Application::cleanup()
{
	delete scene_;
	delete resourceManager_;
	delete engineSettings_;
	delete window_;
	delete renderer_;
}

void Application::run()
{
	float endTime = 0;
	float deltaTime = 0;
	onStartup();
	while (!glfwWindowShouldClose(window_->getGLFWWindow()))
	{
		float start = endTime;
		window_->onUpdate();
		renderer_->prepareFrame();
		onUpdate(deltaTime);
		scene_->update(deltaTime);
		endTime = std::clock();
		deltaTime = endTime - start;
	}
	vkDeviceWaitIdle(renderer_->logicalDevice_);
}
