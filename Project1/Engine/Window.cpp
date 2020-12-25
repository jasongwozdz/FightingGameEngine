#include <functional>
#include <iostream>
#include "Window.h"
#include "EngineSettings.h"

Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	windowInfo.width = EngineSettings::getSingleton().getWindow_width();
	windowInfo.height = EngineSettings::getSingleton().getWindow_height();
	window = glfwCreateWindow(windowInfo.width, windowInfo.height, "Dog Game", nullptr, nullptr);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(window, &windowInfo);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);
		switch (action)
		{
			case GLFW_PRESS:
			{
				Events::KeyPressedEvent k(key);
				info->callback(k);
				break;
			}
			case GLFW_RELEASE:
			{
				Events::KeyReleasedEvent k(key);
				info->callback(k);
				break;
			}
		}
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);

		switch (action)
		{
			case GLFW_PRESS:
			{
				Events::MousePressedEvent k(button);
				info->callback(k);
				break;
			}
			case GLFW_RELEASE:
			{
				Events::MouseReleasedEvent k(button);
				info->callback(k);
				break;
			}
		}
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);

		Events::MouseMoveEvent e(xPos, yPos);
		info->callback(e);
	});
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

GLFWwindow* Window::getGLFWWindow()
{
	return window;
}

void Window::setWindowWidth(float width)
{
	windowInfo.width = width;
}

void Window::setWindowHeight(float height)
{
	windowInfo.height = height;
}

void Window::setCursor(bool cursorEnabled)
{
	if (cursorEnabled)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void Window::onUpdate()
{
	glfwPollEvents();
}

void Window::setEventCallback(const std::function<void(Events::Event&)> f)
{
	windowInfo.callback = f;
}
