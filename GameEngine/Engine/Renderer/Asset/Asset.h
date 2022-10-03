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
	unsigned int numVerticies_;
	unsigned int numInidicies_;
};

struct MeshDataOpengl
{
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
};

class ENGINE_API Asset
{
public:
	friend class AssetInstance;
	friend class VkRenderer;
	friend class OpenGlRenderer;

	Asset();
	~Asset();

	template<typename VertexType>
	void addMesh(std::vector<VertexType> verticies, std::vector<uint32_t> indicies);
	void addTexture(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int numChannels);
	void addSkeleton(int boneStructureIndex);

private:
	//class Renderable* mesh_;
	MeshData mesh_;
	MeshDataOpengl oMesh_;
	struct Textured* texture_;
	class BoneStructure* skeleton_;
};
