#pragma once
#include "Singleton.h"

class EngineSettings : Singleton<EngineSettings>
{
public:
	EngineSettings() = default;

	void init(); // loads in default settings from EngineSettings.ini file

	static EngineSettings& getSingleton();
	static EngineSettings* getSingletonPtr();
	int windowWidth;
	int windowHeight;
	double dt;
	double currentTime;
private:
	const char* fileLoc = "./Engine/Settings/EngineSettings.ini";
};

