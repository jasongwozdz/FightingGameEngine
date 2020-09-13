#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include <stb_image.h>

struct Vertex;

struct TextureReturnVals {
	unsigned char* pixels;
	int textureWidth;
	int textureHeight;
	int textureChannels;
};

class ResourceManager
{

	std::map<std::string, uintptr_t> m_resourceRegistry;

	std::vector<Vertex> loadObjFile(std::string& filePath);
	
	TextureReturnVals* loadTextureFile(std::string& filePath);
};

