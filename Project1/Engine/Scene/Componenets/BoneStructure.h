#pragma once
#include "NewRenderer/Renderable.h"
#include "assimp/scene.h"
#include "glm/glm.hpp"

#include <map>
#include <vector>

struct Joint
{
	glm::mat4 invBindPose_; //inverse bind pose
	std::string name_; //name of joint
	int parent_; //name of parent
	glm::mat4 offset_ = glm::mat4(1.0f); //bone offset
	bool animated_ = false;
};

class BoneStructure
{
public:
	BoneStructure(int numBones);

	BoneStructure(std::vector<Joint> boneInfo);

	//returns index of bone with name.  If not found return -1
	int findBoneByBoneName(std::string boneName);

	std::vector<int> getChildIndicies(std::string parentName);

	glm::mat4 globalInverseTransform_;

	inline int getRootIndex()
	{
		for (int i = 0; i < boneInfo_.size(); i++)
		{
			if (boneInfo_[i].parent_ == -1)
				return i;
		}
	}

	std::vector<Joint> boneInfo_;
};

