#pragma once
#include <string>
#include <vector>


class Textured
{
public:
	Textured(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int textureChannels, std::string entityName);

	std::vector<unsigned char> pixels_;
	int textureWidth_;
	int textureHeight_;
	int textureChannels_;
	std::string entityName_;
};
