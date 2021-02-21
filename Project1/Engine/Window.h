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
	WindowInfo windowInfo;
private:
	GLFWwindow* window;
};

