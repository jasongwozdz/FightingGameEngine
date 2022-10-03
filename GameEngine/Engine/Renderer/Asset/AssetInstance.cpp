#include "AssetInstance.h"

#include "../../EngineSettings.h"
#include "../RendererInterface.h"
#include "../VkRenderer.h"
#include "../OpenGl/OpenGlRenderer.h"

AssetInstance::AssetInstance(Asset* asset) :
	asset_(asset)
{
	//initalize createInfo_ to some default values
	createInfo_.windowExtent.width = EngineSettings::getSingleton().windowWidth;
	createInfo_.windowExtent.height = EngineSettings::getSingleton().windowHeight;
	createInfo_.cullingEnabled = true;
	createInfo_.depthEnabled = true;
	createInfo_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	RendererInterface::getSingleton().setDefaultShader(this);
	init();
}

AssetInstance::AssetInstance(Asset * asset, PipelineCreateInfo createInfo) :
	asset_(asset),
	createInfo_(createInfo)
{
	init();
}

AssetInstance::AssetInstance(AssetInstance&& other)
{
	asset_ = other.asset_;
	data_ = std::move(other.data_);
	other.asset_ = nullptr;
	//other.data_ = nullptr;
	sizeOfUniformData_ = other.sizeOfUniformData_;
	createInfo_ = other.createInfo_;
	shaderProgram_ = other.shaderProgram_;
}

AssetInstance& AssetInstance::operator=(AssetInstance&& other)
{
	asset_ = other.asset_;
	data_ = std::move(other.data_);
	other.asset_ = nullptr;
	sizeOfUniformData_ = other.sizeOfUniformData_;
	createInfo_ = other.createInfo_;
	shaderProgram_ = other.shaderProgram_;
	return *this;
}

AssetInstance::~AssetInstance()
{
}

void AssetInstance::init()
{
	//need to fix this to work with OPENGL
	RendererInterface* renderInterface = RendererInterface::getSingletonPtr();
	if (asset_->skeleton_)
	{
		data_.ubo_ = new MVPBoneData;
		sizeOfUniformData_ = sizeof(MVPBoneData);
		if (renderInterface->api_ == RendererInterface::RenderAPI::VULKAN)
		{
			VkRenderer::getInstance()->uploadDynamicData<MVPBoneData>(this);
		}
		else if (renderInterface->api_ == RendererInterface::RenderAPI::OPENGL)
		{
			OpenGlRenderer::getInstance()->uploadDynamicData<MVPBoneData>(this);
		}
	}
	else
	{
		data_.ubo_ = new MVP;
		sizeOfUniformData_ = sizeof(MVP);
		if (renderInterface->api_ == RendererInterface::RenderAPI::VULKAN)
		{
			VkRenderer::getInstance()->uploadDynamicData<MVP>(this);
		}
		else if (renderInterface->api_ == RendererInterface::RenderAPI::OPENGL)
		{
			OpenGlRenderer::getInstance()->uploadDynamicData<MVP>(this);
		}
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
