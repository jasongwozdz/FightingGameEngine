#pragma once
#include "Singleton.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "../Application.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Input : public Singleton<Input>
{
public:
	Input(class Application* application);
	virtual ~Input();

	//input: axis(horizontal, vertical)
	//return: -1 to 1 
	float getAxis(std::string axis);

	//input button name 
	//return: 0 if not pressed 1 if pressed
	float getButton(std::string button);

	//Adds a new button mapped to keyCode or adds keyCode to existing button
	void addButton(std::string button, int keyCode);

	//Adds low and highKeys to a new axis or existing axis
	void addAxis(std::string axis, int lowKeyCode, int highKeyCode);

	//inherited from Singleton
	static Input& getSingleton();
	static Input* getSingletonPtr();

	glm::vec2 mousePos_;
private:
	void onEvent(Events::Event& event);
	void onKeyPressedEvent(Events::KeyEvent& keyEvent);
	void onKeyReleasedEvent(Events::KeyEvent& keyEvent);
	void onMouseClickEvent(Events::MousePressedEvent& mousePressedEvent);
	void onMouseClickReleasedEvent(Events::MouseReleasedEvent& mouseReleasedEvent);
	void onMouseMovementEvent(Events::MouseMoveEvent& mouseMovedEvent);
	//initalizes all maps using whats in the ini file
	void parseIniFile();
	bool isInputInMap(int keyCode);

private:
	typedef std::pair<int, int> LowHighInputPair;
	//map axis names to pairs of low/high inputs
	std::unordered_map<std::string, std::vector<LowHighInputPair> >axisMap_;
	//maps button names to keycodes
	std::unordered_map<std::string, std::vector<int>> buttonMap_;
	//map of keycodes to their current input
	std::unordered_map<int, float> inputPressedMap_;
	
	//.ini parsing vars
	const char* INI_LOCATION     = "./Engine/Settings/KeyMap.ini";
	const char* INI_AXIS_SECTION = "Axis";
	const char* INI_BUTTON_SECTION = "Button";
	std::vector<std::string> iniButtonNames = 
	{
		"Fire1",
		"Fire2",
		"Fire3",
		"LeftClick",
		"RightClick"
	};
	std::vector<std::string> iniAxisNames = 
	{
		"Vertical",
		"Horizontal"
	};
};
