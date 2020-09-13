#include "ResourceManager.h"
#include "BufferOperations.h"
#include "Vertex.h"
/*
should write a load texture file that loads in textures pixel data given a path to it.

Should also handle referential integrity between game objects

*/



std::vector<Vertex> ResourceManager::loadObjFile(std::string& filePath) {
	std::vector<Vertex> dog;
	return dog;
}

TextureReturnVals* ResourceManager::loadTextureFile(std::string& filePath) {
	TextureReturnVals* returnVals;
	//Look in resourceRegistry for texture.  If its not found allocate resources and then add to registry.
	//If it is found grab pointer from registry
	auto find = m_resourceRegistry.find(filePath);
	if (find == m_resourceRegistry.end())
	{
		int texWidth, texHeight, texChannels;
		unsigned char* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		returnVals = new TextureReturnVals;
		*returnVals = { pixels, texWidth, texHeight, texChannels };

		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(returnVals);
	}
	else
	{
		returnVals = reinterpret_cast<TextureReturnVals*>(find->second);
	}

	return returnVals;
}
