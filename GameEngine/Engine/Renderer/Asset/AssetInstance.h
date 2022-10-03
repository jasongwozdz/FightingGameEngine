#pragma once

#include <string>
#include <memory>

#include "../VkTypes.h"
#include "AssetTypes.h"
#include "Asset.h"
#include "../../EngineExport.h"

class ENGINE_API AssetInstance
{
public:
	friend class VkRenderer;
	friend class OpenGlRenderer;

	AssetInstance() = delete;
	AssetInstance(Asset* asset);
	AssetInstance(Asset* asset, PipelineCreateInfo createInfo);
	AssetInstance(const AssetInstance& other) = delete;
	AssetInstance(AssetInstance&& other);
	AssetInstance& operator=(AssetInstance&& other);
	~AssetInstance();

	void setVertexShader(std::string shaderPath);
	void setFragmentShader(std::string shaderPath);
	void setModelMatrix(glm::mat4 modelToWorld);
	void setViewMatrix(glm::mat4 worldToView);
	void setProjectionMatrix(glm::mat4 viewToClip);
	void setPose(std::vector<glm::mat4> bones);
	const BoneStructure* getSkeleton() const { return asset_->skeleton_; };

	bool toDelete = false;
private:
	void init();

	Asset* asset_;
	DynamicAssetData data_;
	size_t sizeOfUniformData_;
	PipelineCreateInfo createInfo_;

	unsigned int shaderProgram_;
};

