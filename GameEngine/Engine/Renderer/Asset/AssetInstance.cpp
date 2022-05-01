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
	createInfo_.vertexShader = "./shaders/texturedMeshVert.spv";
	createInfo_.fragmentShader = "./shaders/texturedMeshFrag.spv";
	createInfo_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	if (asset->skeleton_)
	{
		createInfo_.hasSkeleton = true;
		if (!createInfo_.lightingEnabled)
		{
			createInfo_.vertexShader = "./shaders/animatedMesh.vert.spv";
			createInfo_.fragmentShader = "./shaders/animatedMesh.frag.spv";
		}
		else
		{
			createInfo_.vertexShader = "./shaders/animatedMeshLighting.vert.spv";
			createInfo_.fragmentShader = "./shaders/animatedMeshLighting.frag.spv";
		}
	}
	createInfo_.hasTexture = asset->texture_;
	init();
}

AssetInstance::AssetInstance(AssetInstance&& other)
{
	asset_ = other.asset_;
	data_ = std::move(other.data_);
	sizeOfUniformData_ = other.sizeOfUniformData_;
	createInfo_ = other.createInfo_;
}

AssetInstance& AssetInstance::operator=(AssetInstance&& other)
{
	asset_ = other.asset_;
	other.asset_ = nullptr;
	data_ = std::move(other.data_);
	sizeOfUniformData_ = other.sizeOfUniformData_;
	createInfo_ = other.createInfo_;
	return *this;
}

void AssetInstance::init()
{
	VkRenderer* renderer = VkRenderer::getSingletonPtr();
	if (asset_->skeleton_)
	{
		data_.ubo_ = new MVPBoneData;
		sizeOfUniformData_ = sizeof(MVPBoneData);
		renderer->uploadDynamicData<MVPBoneData>(this);
	}
	else
	{
		data_.ubo_ = new MVP;
		sizeOfUniformData_ = sizeof(MVP);
		renderer->uploadDynamicData<MVP>(this);
	}
}

void AssetInstance::setVertexShader(std::string shaderPath)
{
	createInfo_.vertexShader = shaderPath;
}

void AssetInstance::setFragmentShader(std::string shaderPath)
{
	createInfo_.fragmentShader = shaderPath;
}

void AssetInstance::setModelMatrix(glm::mat4 modelToWorld)
{
	data_.ubo_->model = modelToWorld;
}

void AssetInstance::setViewMatrix(glm::mat4 worldToView)
{
	data_.ubo_->view = worldToView;
}

void AssetInstance::setProjectionMatrix(glm::mat4 viewToClip)
{
	data_.ubo_->projection = viewToClip;
}

void AssetInstance::setPose(std::vector<glm::mat4> bones)
{
	if (asset_->skeleton_)
	{
		MVPBoneData* mvpData = static_cast<MVPBoneData*>(data_.ubo_);
		for (int i = 0; i < bones.size(); i++)
		{
			mvpData->bones_[i] = bones[i];
		}
	}
	else
	{
		_ASSERT_EXPR(false, "ERROR: Can't set pose for asset instance with no skeleton");
	}
}
