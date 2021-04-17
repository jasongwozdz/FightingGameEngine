#pragma once
#include <GLFW/glfw3.h>
#include "Events.h"

struct WindowInfo
{
	float width;
	float height;
	std::function<void(Events::Event&)> callback;
};

class Window
{
public:
	Window();
	~Window();
	GLFWwindow* getGLFWWindow();
	void setWindowWidth(float width);
	void setWindowHeight(float height);
	void setCursor(bool cursorEnabled);
	void onUpdate();
	void setEventCallback(const std::function<void(Events::Event&)> e);
	void setWindowResizeCallback(const std::function<void(GLFWwindow* window, int width, int height)> fn);

	static float getTime()
	{
		return glfwGetTime();
	}

	WindowInfo windowInfo_;
private:
	GLFWwindow* window_;
};

