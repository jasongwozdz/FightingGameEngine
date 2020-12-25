#pragma once
#include "Singleton.h"

class EngineSettings : Singleton<EngineSettings>
{
public:
	EngineSettings() = default;

	void init(); // loads in default settings from EngineSettings.ini file

	int getWindow_width();
	int getWindow_height();
	void setWindow_width(int width);
	void setWindow_height(int height);

	static EngineSettings& getSingleton();
	static EngineSettings* getSingletonPtr();
private:
	const char* fileLoc = "./Engine/Settings/EngineSettings.ini";
	int window_width;
	int window_height;
};

