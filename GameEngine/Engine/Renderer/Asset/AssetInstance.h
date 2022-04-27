#pragma once

#include <string>

#include "../VkTypes.h"
#include "AssetTypes.h"
#include "Asset.h"
#include "../../EngineExport.h"


class ENGINE_API AssetInstance
{
public:
	friend class VkRenderer;

	AssetInstance(Asset* asset);
	AssetInstance(const AssetInstance& other) = delete;
	AssetInstance(AssetInstance&& other);
	AssetInstance& operator=(AssetInstance&& other);
	virtual ~AssetInstance() = default;

	void setVertexShader(std::string shaderPath);
	void setFragmentShader(std::string shaderPath);
	void setModelMatrix(glm::mat4 modelToWorld);
	void setViewMatrix(glm::mat4 worldToView);
	void setProjectionMatrix(glm::mat4 viewToClip);
	void setPose(std::vector<glm::mat4> bones);
	const BoneStructure* getSkeleton() { return asset_->skeleton_; };

private:
	void init();

	Asset* asset_;
	DynamicAssetData data_;
	size_t sizeOfUniformData_;
	PipelineCreateInfo createInfo_;
};

