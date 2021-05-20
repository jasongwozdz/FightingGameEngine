#include "Textured.h"


Textured::Textured(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int textureChannels, std::string entityName) :
	pixels_(pixels), textureWidth_(textureWidth), textureHeight_(textureHeight), textureChannels_(textureChannels)
{};
