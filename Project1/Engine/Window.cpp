#include <functional>
#include <iostream>
#include "Window.h"
#include "EngineSettings.h"
#include "../Engine/libs/imgui/imgui_impl_glfw.h"

Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	windowInfo_.width = EngineSettings::getSingleton().getWindow_width();
	windowInfo_.height = EngineSettings::getSingleton().getWindow_height();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window_= glfwCreateWindow(windowInfo_.width, windowInfo_.height, "Dog Game", nullptr, nullptr);
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(window_, &windowInfo_);

	glfwSetScrollCallback(window_, ImGui_ImplGlfw_ScrollCallback);

	glfwSetCharCallback(window_, ImGui_ImplGlfw_CharCallback);

	glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
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

	glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

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

	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xPos, double yPos)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);

		Events::MouseMoveEvent e(xPos, yPos);
		info->callback(e);
	});

	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);
		info->width = width;
		info->height = height;

		Events::FrameBufferResizedEvent e(width, height);
		info->callback(e);
	});
}

Window::~Window()
{
	glfwDestroyWindow(window_);
	glfwTerminate();
}

GLFWwindow* Window::getGLFWWindow()
{
	return window_;
}

void Window::setWindowWidth(float width)
{
	windowInfo_.width = width;
}

void Window::setWindowHeight(float height)
{
	windowInfo_.height = height;
}

void Window::setCursor(bool cursorEnabled)
{
	if (cursorEnabled)
	{
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else
	{
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void Window::onUpdate()
{
	glfwPollEvents();
}

void Window::setEventCallback(const std::function<void(Events::Event&)> f)
{
	windowInfo_.callback = f;
}
