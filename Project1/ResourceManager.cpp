#include "ResourceManager.h"
#include "BufferOperations.h"
#include "Vertex.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


/*
should write a load texture file that loads in textures pixel data given a path to it.

Should also handle referential integrity between game objects

*/

template<> ResourceManager* Singleton<ResourceManager>::msSingleton = 0;

ResourceManager* ResourceManager::getSingletonPtr()
{
	return msSingleton;
}

ResourceManager& ResourceManager::getSingleton()
{
	assert(msSingleton);  return (*msSingleton);
}


namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

ModelReturnVals& ResourceManager::loadObjFile(std::string& filePath) {
	auto find = m_resourceRegistry.find(filePath);
	ModelReturnVals* vals;
	if (find == m_resourceRegistry.end())
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				
				indices.push_back(uniqueVertices[vertex]);
			}
		}
		vals = new ModelReturnVals;
		vals->vertices = vertices;
		vals->indices = indices;
		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(vals);
	}
	else
	{
		vals = reinterpret_cast<ModelReturnVals*>(m_resourceRegistry[filePath]);
	}
	return *vals;
}

TextureReturnVals& ResourceManager::loadTextureFile(std::string& filePath)
{
	TextureReturnVals* returnVals;
	//Look in resourceRegistry for texture.  If its not found, allocate resources and then add to registry.
	//If it is found grab pointer from registry
	auto find = m_resourceRegistry.find(filePath);
	if (find == m_resourceRegistry.end())
	{
		int texWidth, texHeight, texChannels;
		unsigned char* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cout << "invalid texture path" << std::endl;
			assert(1 == 0);
		}

		returnVals = new TextureReturnVals{ pixels, texWidth, texHeight, texChannels };

		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(returnVals);
	}
	else
	{
		returnVals = reinterpret_cast<TextureReturnVals*>(find->second);
	}

	return *returnVals;
}

void ResourceManager::freeResource(std::string filePath)
{
	auto find = m_resourceRegistry.find(filePath);
	if (find == m_resourceRegistry.end())
	{
		std::cout << "Resource not in registry" << std::endl;
		assert(1 == 0);
	}
	else
	{
		delete reinterpret_cast<void*>(m_resourceRegistry[filePath]);
	}
}

void ResourceManager::freeAllResources() 
{
	for (auto const& resource : m_resourceRegistry)
	{
		delete reinterpret_cast<void*>(resource.second);
	}
}
