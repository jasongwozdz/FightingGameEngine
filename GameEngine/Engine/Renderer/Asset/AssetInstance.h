#pragma once

#include <string>

#include "../VkTypes.h"
#include "AssetTypes.h"
#include "Asset.h"


class AssetInstance
{
public:
	friend class VkRenderer;

	AssetInstance(Asset* asset);

	virtual ~AssetInstance() = default;
	void initalizeDynamicData();
	void setVertexShader(std::string shaderPath);
	void setFragmentShader(std::string shaderPath);

private:
	Asset* asset_;
	DynamicAssetData* data_;
	PipelineCreateInfo createInfo_;
	std::string vertexShader_;
	std::string fragmentShader_;
};

