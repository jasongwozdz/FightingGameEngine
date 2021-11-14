#include <ctime>
#include <chrono>
#include <thread>

#include "Application.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//return in milliseconds
double getCurrentTime()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	auto nano = ms.time_since_epoch();
	return nano.count();
}

Application::Application()
{
	resourceManager_ = new ResourceManager();
	engineSettings_ = new EngineSettings();
	engineSettings_->init();
	window_ = new Window();
	window_->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	renderer_ = new VkRenderer(*window_);
	renderer_->init();
	scene_ = Scene::getSingletonPtr();
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
	dispatcher.dispatch<Events::FrameBufferResizedEvent>(std::bind(&Scene::frameBufferResizedCallback, scene_, std::placeholders::_1));

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
	double endTime = getCurrentTime();
	double deltaTime = 0;
	double start = 0;
	onStartup();
	while (!glfwWindowShouldClose(window_->getGLFWWindow()))
	{
		endTime = getCurrentTime();
		deltaTime = static_cast<float>(endTime - start);//get time in milliseconds
		if (deltaTime >= 1000 / 144)
		{
			start = endTime;
			window_->onUpdate();
			renderer_->prepareFrame();
			onUpdate(deltaTime);
			scene_->update(deltaTime);
		}
	}
	vkDeviceWaitIdle(renderer_->logicalDevice_);
}
