#include "EngineSettings.h"
#include <string>
#include <windows.h>
#include <cwchar>
#include <assert.h>


template <> EngineSettings* Singleton<EngineSettings>::msSingleton = 0;


EngineSettings* EngineSettings::getSingletonPtr()
{
	return msSingleton;
}

EngineSettings& EngineSettings::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

void EngineSettings::init()
{
	int size;
	const char* section = "window";
	const char* widthStr = "width";
	const char* heightStr = "height";
	windowWidth = GetPrivateProfileIntA(section, widthStr, NULL, fileLoc);
	windowHeight = GetPrivateProfileIntA(section, heightStr, NULL, fileLoc);
}
