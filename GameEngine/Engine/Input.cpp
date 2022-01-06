#include "Input.h"

#include <windows.h>
#include <iostream>

#include "../Application.h"

template <> Input* Singleton<Input>::msSingleton = 0;

Input* Input::getSingletonPtr()
{
	return msSingleton;
}

Input& Input::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

Input::Input(Application * application)
{
	application->addEventCallback(ENGINE_EVENT_CALLBACK(Input::onEvent));
	parseIniFile();
	Input::msSingleton = this;
}

Input::~Input()
{
}

float Input::getAxis(std::string axis)
{
	float pressedValue = 0.0f;
	auto entry = axisMap_.find(axis);
	if(entry != axisMap_.end())
	{
		std::vector<LowHighInputPair> axisInputs = entry->second;
		for (auto inputs : axisInputs)
		{
			if (isInputInMap(inputs.first))//low input
			{
				pressedValue -= inputPressedMap_[inputs.first];
			}
			if (isInputInMap(inputs.second))//high input
			{
				pressedValue += inputPressedMap_[inputs.second];
			}
		}
	}
	else
	{
		std::cout << "ERROR couldn't find axis" << std::endl;
	}
	return std::clamp(pressedValue, -1.0f, 1.0f);
}

float Input::getButton(std::string button)
{
	auto entry = buttonMap_.find(button);
	if (entry != buttonMap_.end())
	{
		std::vector<int> buttonKeyCodes = entry->second;
		for (int keyCode : buttonKeyCodes)
		{
			if (isInputInMap(keyCode))
			{
				return std::clamp(inputPressedMap_[keyCode], 0.0f, 1.0f);
			}
		}
	}
	else
	{
		std::cout << "ERROR couldn't find button" << std::endl;
	}
	return 0.0f;
}

void Input::addButton(std::string button, int keyCode)
{

}

void Input::addAxis(std::string axis, int lowKeyCode, int highKeyCode)
{
	auto entry = axisMap_.find(axis);
	if (entry != axisMap_.end())
	{
		axisMap_[axis].push_back(std::pair<int, int>(lowKeyCode, highKeyCode));
	}
	else
	{
		std::vector<LowHighInputPair> keyCodes(1, {lowKeyCode, highKeyCode});
		axisMap_.insert({ axis, keyCodes});
	}
	inputPressedMap_.insert({ lowKeyCode , 0.0f });
	inputPressedMap_.insert({ highKeyCode, 0.0f });
}

void Input::onEvent(Events::Event & event)
{
	static int calls = 0;
	Events::EventDispatcher dispatcher(event);

	dispatcher.dispatch<Events::KeyPressedEvent>(ENGINE_EVENT_CALLBACK(Input::onKeyPressedEvent));
	dispatcher.dispatch<Events::KeyReleasedEvent>(ENGINE_EVENT_CALLBACK(Input::onKeyReleasedEvent));
	dispatcher.dispatch<Events::MouseMoveEvent>(ENGINE_EVENT_CALLBACK(Input::onMouseMovementEvent));
	dispatcher.dispatch<Events::MousePressedEvent>(ENGINE_EVENT_CALLBACK(Input::onMouseClickEvent));
	dispatcher.dispatch<Events::MouseReleasedEvent>(ENGINE_EVENT_CALLBACK(Input::onMouseClickReleasedEvent));
	calls++;
}

void Input::onKeyPressedEvent(Events::KeyEvent & keyEvent)
{
	if (isInputInMap(keyEvent.KeyCode))
	{
		inputPressedMap_[keyEvent.KeyCode] = 1.0f;
	}
}

void Input::onKeyReleasedEvent(Events::KeyEvent & keyEvent)
{
	if (isInputInMap(keyEvent.KeyCode))
	{
		inputPressedMap_[keyEvent.KeyCode] = 0.0f;
	}
}

void Input::onMouseClickEvent(Events::MousePressedEvent & mousePressedEvent)
{
	if (isInputInMap(mousePressedEvent.mouseCode))
	{
		inputPressedMap_[mousePressedEvent.mouseCode] = 1.0f;
	}
}

void Input::onMouseClickReleasedEvent(Events::MouseReleasedEvent & mouseReleasedEvent)
{
	if (isInputInMap(mouseReleasedEvent.mouseCode))
	{
		inputPressedMap_[mouseReleasedEvent.mouseCode] = 0.0f;
	}
}

void Input::onMouseMovementEvent(Events::MouseMoveEvent & mouseMovedEvent)
{
	mousePos_ = { mouseMovedEvent.mouseX, mouseMovedEvent.mouseY };
}

void Input::parseIniFile()
{
	//read in buttons
	for (std::string& button : iniButtonNames)
	{
		std::vector<int> keys;
		int keyCode = GetPrivateProfileIntA(INI_BUTTON_SECTION, button.c_str(), NULL, INI_LOCATION);
		if (keyCode == 0)
		{
			std::cout << "ERROR loading keys: ini file is probably not missing.  Need to copy KeyMap.ini to Engine/Settings from your root directory" << std::endl;
		}
		keys.push_back(keyCode);
		buttonMap_.insert({ button, keys });
		for (int key : keys)
		{
			inputPressedMap_.insert({ key, 0.0f });
		}
	}

	//read axis inputs
	for (std::string& axis : iniAxisNames)
	{
		std::vector<LowHighInputPair> axisInputs;
		LowHighInputPair lowHighInput;
		lowHighInput.first = GetPrivateProfileIntA(axis.c_str(), "Low", NULL, INI_LOCATION);
		lowHighInput.second = GetPrivateProfileIntA(axis.c_str(), "High", NULL, INI_LOCATION);
		axisInputs.push_back(lowHighInput);
		axisMap_.insert({ axis, axisInputs });
		for (LowHighInputPair& pair : axisInputs)
		{
			inputPressedMap_.insert({ pair.first,  0.0f });
			inputPressedMap_.insert({ pair.second, 0.0f });
		}
	}
}

inline bool Input::isInputInMap(int keyCode)
{
	auto entry = inputPressedMap_.find(keyCode);
	if (entry != inputPressedMap_.end())
	{
		return true;
	}
	return false;
}
