#pragma once
#include "assimp/scene.h"
#include "Scene/Componenets/BoneStructure.h"
#include <vector>

class Animator
{
public:

	Animator(Animator&& animator);

	Animator& operator=(Animator const& other)
	{
		this->scene_ = other.scene_;
		this->boneStructure_ = other.boneStructure_;
		this->globalInverseTransform_ = (other.scene_->mRootNode->mTransformation),
		this->animationSpeed_ = (other.animationSpeed_);
		this->runningTime_ = (other.runningTime_);
		this->currentAnimation_ = (other.currentAnimation_);
		this->boneTransforms_ = (other.boneTransforms_);
		return *this;
	}

	Animator(aiScene* scene, BoneStructure& boneStructure);

	std::vector<aiMatrix4x4> update(float deltaTime);

	bool playAnimation(std::string animationName);

	aiScene* scene_;

	float startTime_ = 0;

	float runningTime_;

	float animationSpeed_;

	int currentAnimation_ = -1;

	BoneStructure& boneStructure_;

	std::vector<aiMatrix4x4> boneTransforms_;

	aiMatrix4x4 globalInverseTransform_;

	void readNodeHierarchy(float animationTime, aiNode* pNode, aiMatrix4x4 parentTransform);

	const aiNodeAnim* findNodeAnim(std::string nodeName);

	aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim);

	aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim);

	aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim);
};
