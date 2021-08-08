#include "BoneStructure.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

BoneStructure::BoneStructure(std::vector<Joint> boneInfo) :
	boneInfo_(boneInfo)
{}

BoneStructure::BoneStructure(int numBones)
{
	boneInfo_.resize(numBones);
}

int BoneStructure::findBoneByBoneName(std::string boneName)
{
	int i = 0;
	for(Joint j : boneInfo_)
	{
		if (boneName == j.name_)
			return i;
		i++;
	}
	return -1;
}

std::vector<int> BoneStructure::getChildIndicies(std::string parentName)
{
	int parentIndex = findBoneByBoneName(parentName);
	std::vector<int> indicies;
	for (int i = 0; i < boneInfo_.size(); i++)
	{
		if (boneInfo_[i].parent_ == parentIndex)
		{
			indicies.push_back(i);
		}
	}
	return indicies;
}
