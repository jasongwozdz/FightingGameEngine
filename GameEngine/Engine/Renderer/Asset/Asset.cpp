#include "Asset.h"

#include "../../ResourceManager.h"
#include "../VkRenderer.h"
#include "../Renderable.h"
#include "../Textured.h"

Asset::Asset() :
	mesh_(nullptr),
	texture_(nullptr),
	skeleton_(nullptr)
{};

Asset::~Asset()
{
	delete mesh_;// this is where deleting memory from gpu will happen
	delete texture_; // this is where deleting memory from gpu will happen
	 delete skeleton_;
}

void Asset::addMesh(std::vector<Vertex> verticies, std::vector<uint32_t> indicies)
{
	mesh_ = new Renderable(verticies, indicies, true, "", true);
	VkRenderer* renderer = VkRenderer::getSingletonPtr();
	renderer->uploadStaticMeshData(mesh_);
}

void Asset::addTexture(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int numChannels)
{
	texture_ = new Textured(pixels, textureWidth, textureHeight, numChannels);
	VkRenderer* renderer = VkRenderer::getSingletonPtr();
	renderer->uploadTextureData(texture_);
}

void Asset::addSkeleton(int boneStructureIndex)
{
	ResourceManager* resourceManager = ResourceManager::getSingletonPtr();
	_ASSERT(boneStructureIndex < resourceManager->boneStructures_.size());
	skeleton_ = &resourceManager->boneStructures_[boneStructureIndex];
}
