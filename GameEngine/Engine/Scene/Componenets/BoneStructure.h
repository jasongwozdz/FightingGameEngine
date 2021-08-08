#pragma once
#include "assimp/scene.h"
#include "glm/glm.hpp"
#include "../../NewRenderer/Renderable.h"

#include <map>
#include <vector>

struct Joint
{
	glm::mat4 invBindPose_; //inverse bind pose
	glm::mat4 offset_ = glm::mat4(1.0f); //bone offset
	int parent_; //name of parent
	bool animated_ = false;
	std::string name_; //name of joint
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

