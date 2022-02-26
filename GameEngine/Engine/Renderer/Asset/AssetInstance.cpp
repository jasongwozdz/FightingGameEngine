#include "AssetInstance.h"

#include "../../EngineSettings.h"

#include "../VkRenderer.h"

AssetInstance::AssetInstance(Asset* asset) :
	asset_(asset)
{
	//initalize createInfo_ to some default values
	createInfo_.windowExtent.width = EngineSettings::getSingleton().windowWidth;
	createInfo_.windowExtent.height = EngineSettings::getSingleton().windowHeight;
	createInfo_.cullingEnabled = true;
	createInfo_.depthEnabled = true;
	createInfo_.fragmentShader = "./shaders/texturedMeshVert.spv";
	createInfo_.vertexShader = "./shaders/texturedMeshFrag.spv";
	createInfo_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	if (asset->skeleton_)
	{
		createInfo_.hasSkeleton = true;
		createInfo_.fragmentShader = "./shaders/animatedMesh.vert.spv";
		createInfo_.vertexShader = "./shaders/animatedMesh.frag.spv";
	}
	createInfo_.hasTexture = asset->texture_;
};

void AssetInstance::initalizeDynamicData()
{
	VkRenderer* renderer = VkRenderer::getSingletonPtr();
	renderer->uploadDynamicData<MVP>(this);
}
