#pragma once
#include <vector>

#include "../../EngineExport.h"
#include "../../Vertex.h"
#include "../../Scene/Components/BoneStructure.h"
#include "../VkTypes.h"

struct MeshData
{
	VulkanBuffer indexBuffer_;
	VulkanBuffer vertexBuffer_;
	int numVerticies_;
	int numInidicies_;
};

class ENGINE_API Asset
{
public:
	friend class AssetInstance;
	friend class VkRenderer;

	Asset();
	~Asset();

	template<typename VertexType>
	void addMesh(std::vector<VertexType> verticies, std::vector<uint32_t> indicies);
	void addTexture(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int numChannels);
	void addSkeleton(int boneStructureIndex);

private:
	//class Renderable* mesh_;
	MeshData mesh_;
	class Textured* texture_;
	class BoneStructure* skeleton_;
};
