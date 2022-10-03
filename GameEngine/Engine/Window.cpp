#include <functional>
#include <iostream>
#include "Window.h"
#include "EngineSettings.h"
#include "../Engine/libs/imgui/imgui_impl_glfw.h"
#include "Renderer/RendererInterface.h"

Window::Window()
{
	glfwInit();
	windowInfo_.width = EngineSettings::getSingleton().windowWidth;
	windowInfo_.height = EngineSettings::getSingleton().windowHeight;
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	if (EngineSettings::getSingleton().isOpenglApi())
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	else if(EngineSettings::getSingleton().isVulkanApi())
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	window_= glfwCreateWindow(windowInfo_.width, windowInfo_.height, "Engine", nullptr, nullptr);

	if (EngineSettings::getSingleton().isOpenglApi())
	{
		glfwMakeContextCurrent(getGLFWWindow());
	}

	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

	glfwSetScrollCallback(window_, [](GLFWwindow* window, double xOffset, double yOffset) {
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);

		Events::MouseScrolledEvent e(xOffset, yOffset);
		info->callback(e);
	});

	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height)
	{
		WindowInfo* info = (WindowInfo*)glfwGetWindowUserPointer(window);
		info->width = width;
		info->height = height;
		EngineSettings::getSingletonPtr()->windowHeight = info->height;
		EngineSettings::getSingletonPtr()->windowWidth = info->width;

		Events::FrameBufferResizedEvent e(width, height);
		info->callback(e);
	});
}

Window::~Window()
{
	glfwDestroyWindow(window_);
	glfwTerminate();
}


float Window::getTime()
{
	return glfwGetTime();
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
	if (EngineSettings::getSingleton().isOpenglApi())
	{
		glfwSwapBuffers(getGLFWWindow());
	}
	glfwPollEvents();
}

void Window::setEventCallback(const std::function<void(Events::Event&)> f)
{
	windowInfo_.callback = f;
}
