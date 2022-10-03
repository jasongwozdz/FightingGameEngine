#pragma once
#include "Singleton.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API EngineSettings : Singleton<EngineSettings>
{
public:
	EngineSettings() = default;

	void init(); // loads in default settings from EngineSettings.ini file

	static EngineSettings& getSingleton();
	static EngineSettings* getSingletonPtr();

	bool isVulkanApi();
	bool isOpenglApi();

	int windowWidth;
	int windowHeight;
	int framesPerSecond = 144;
	double dt;
	double currentTime;
	int renderApi;
private:
	const char* fileLoc = "./Engine/Settings/EngineSettings.ini";
};

