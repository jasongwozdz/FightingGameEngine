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
	window_width = GetPrivateProfileIntA(section, widthStr, NULL, fileLoc);
	window_height = GetPrivateProfileIntA(section, heightStr, NULL, fileLoc);
}

int	EngineSettings::getWindow_width()
{
	return window_width;
}

int EngineSettings::getWindow_height()
{
	return window_height;
}

void EngineSettings::setWindow_width(int width)
{
	window_width = width;
}

void EngineSettings::setWindow_height(int height)
{
	window_height = height;
}
