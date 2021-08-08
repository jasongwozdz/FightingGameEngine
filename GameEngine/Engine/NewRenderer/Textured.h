#pragma once
#include <string>
#include <vector>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Textured
{
public:
	Textured(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int textureChannels, std::string entityName);

	std::vector<unsigned char> pixels_;
	int textureWidth_;
	int textureHeight_;
	int textureChannels_;
	std::string entityName_;
};
