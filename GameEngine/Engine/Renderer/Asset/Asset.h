#pragma once
#include <vector>

#include "../../Vertex.h"
#include "../../Scene/Components/BoneStructure.h"

class Asset
{
public:
	friend class AssetInstance;
	friend class VkRenderer;

	Asset();
	~Asset();

private:
	void addMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies);
	void addTexture(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int numChannels);
	void addSkeleton(int boneStructureIndex);

private:
	class Renderable* mesh_;
	class Textured* texture_;
	class BoneStructure* skeleton_;
};
