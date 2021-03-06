#pragma once
#include "NewRenderer/Renderable.h"
#include "assimp/scene.h"
#include "ResourceManager.h"

#include <map>
#include <vector>


class BoneStructure
{
public:
	BoneStructure(std::map<std::string, uint32_t> boneMapping, std::vector<BoneInfo> boneInfo);

	BoneStructure& operator=(BoneStructure const& other)
	{
		this->boneMapping_ = other.boneMapping_;
		this->boneInfo_ = other.boneInfo_;
		return *this;
	}

	void setPose(std::vector<aiMatrix4x4>& pose, Renderable& renderable);

	std::map<std::string, uint32_t> boneMapping_;
	std::vector<BoneInfo> boneInfo_;
};

