#pragma once
#include "Singleton.h"
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

struct ModelReturnVals {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class ResourceManager : Singleton<ResourceManager>
{
public:

	std::map<std::string, uintptr_t> m_resourceRegistry;

	ModelReturnVals& loadObjFile(std::string& filePath);
	
	TextureReturnVals& loadTextureFile(std::string& filePath);

	void freeResource(std::string filePath);

	void freeAllResources();

	static ResourceManager& getSingleton();

	static ResourceManager* getSingletonPtr();

};

