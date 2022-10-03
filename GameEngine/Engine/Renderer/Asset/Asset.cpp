#include "Asset.h"

#include "../../ResourceManager.h"
#include "../VkRenderer.h"
#include "../Renderable.h"
#include "../Textured.h"
#include "../OpenGl/OpenGlRenderer.h"

Asset::Asset() :
	//mesh_(nullptr),
	texture_(nullptr),
	skeleton_(nullptr)
{};

Asset::~Asset()
{
	//delete mesh_;// this is where deleting memory from gpu will happen
	delete texture_; // this is where deleting memory from gpu will happen
	delete skeleton_;
}

template<typename VertexType>
void Asset::addMesh(std::vector<VertexType> verticies, std::vector<uint32_t> indicies)
{
	mesh_.numInidicies_ = indicies.size();
	mesh_.numVerticies_ = verticies.size();
	RendererInterface* renderInterface = RendererInterface::getSingletonPtr();
	if(renderInterface->api_ == RendererInterface::RenderAPI::VULKAN)
	{
		VkRenderer* renderer = static_cast<VkRenderer*>(RendererInterface::getSingletonPtr());
		renderer->uploadStaticMeshData(verticies, indicies, &mesh_.vertexBuffer_, &mesh_.indexBuffer_);
	}
	else if(renderInterface->api_ == RendererInterface::RenderAPI::OPENGL)
	{
		OpenGlRenderer* renderer = OpenGlRenderer::getInstance();
		renderer->uploadStaticMeshData(this, verticies, indicies);

		//_ASSERTE(false, ("Need to handle this for OpenGl"));
	}
}

template void Asset::addMesh<NonAnimVertex>(std::vector<NonAnimVertex> verticies, std::vector<uint32_t> indicies);

template void Asset::addMesh<Vertex>(std::vector<Vertex> verticies, std::vector<uint32_t> indicies);

void Asset::addTexture(std::vector<unsigned char> pixels, int textureWidth, int textureHeight, int numChannels)
{
	texture_ = new Textured(pixels, textureWidth, textureHeight, numChannels);
	RendererInterface* renderer = RendererInterface::getSingletonPtr();
	renderer->uploadTextureData(texture_);
}

void Asset::addSkeleton(int boneStructureIndex)
{
	ResourceManager* resourceManager = ResourceManager::getSingletonPtr();
	_ASSERT(boneStructureIndex < resourceManager->boneStructures_.size());
	skeleton_ = &resourceManager->boneStructures_[boneStructureIndex];
}
