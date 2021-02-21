#pragma once
#include "NewRenderer/Renderable.h"
#include "assimp/scene.h"
#include "ResourceManager.h"

#include <map>
#include <vector>


class BoneStructure
{
public:
	BoneStructure(std::map<std::string, uint32_t> boneMapping, std::vector<BoneInfo> boneInfo, Renderable& renderable);

	BoneStructure& operator=(BoneStructure const& other)
	{
		this->renderable_ = other.renderable_;
		this->scene_ = other.scene_;
		this->boneMapping_ = other.boneMapping_;
		this->boneInfo_ = other.boneInfo_;
		this->rootBone_ = other.rootBone_;
		return *this;
	}

	void setPose(std::vector<aiMatrix4x4> pose);

	Renderable& renderable_;
	aiScene* scene_;
	std::map<std::string, uint32_t> boneMapping_;
	std::vector<BoneInfo> boneInfo_;
	aiBone* rootBone_;
};

