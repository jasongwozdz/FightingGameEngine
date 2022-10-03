#include "EngineSettings.h"
#include <string>
#include <windows.h>
#include <cwchar>
#include <assert.h>
#include <iostream>

#include "Renderer/RendererInterface.h"


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

	if (windowWidth <= 0 || windowHeight <= 0)
	{
		std::cout << "ERROR: window with/height are 0.  This could mean the Engine settings file was not found.  Defaulting to 640 x 480 width/height" << std::endl;
		windowWidth = 640;
		windowHeight = 480;
	}
	
	section = "graphics";
	const char* apiStr = "api";
	renderApi = GetPrivateProfileIntA(section, apiStr, NULL, fileLoc);
}

bool EngineSettings::isVulkanApi()
{
	return static_cast<RendererInterface::RenderAPI>(renderApi) == RendererInterface::RenderAPI::VULKAN;
}

bool EngineSettings::isOpenglApi()
{
	return static_cast<RendererInterface::RenderAPI>(renderApi) == RendererInterface::RenderAPI::OPENGL;
}

