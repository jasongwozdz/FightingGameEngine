#pragma once
#include "Events.h"
#include <GLFW/glfw3.h>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

struct GLFWwindow;

struct WindowInfo
{
	float width;
	float height;
	std::function<void(Events::Event&)> callback;
};

class ENGINE_API Window
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

	static float getTime();

	WindowInfo windowInfo_;
private:
	GLFWwindow* window_;
};

