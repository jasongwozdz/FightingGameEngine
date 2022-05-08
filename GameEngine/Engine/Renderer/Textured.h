#pragma once
#include <string>
#include <vector>

#include "VkTypes.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

struct ENGINE_API Textured
{
	Textured(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int textureChannels) :
		pixels_(pixels),
		textureWidth_(textureWidth),
		textureHeight_(textureHeight),
		textureChannels_(textureChannels)
	{};

	std::vector<unsigned char> pixels_;
	TextureResources resources_;
	int textureWidth_;
	int textureHeight_;
	int textureChannels_;
};
