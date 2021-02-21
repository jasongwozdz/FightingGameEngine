#include <iostream>
#include <fstream>
#include "Animator.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

Animator::Animator(Animator&& animator) :
	boneStructure_(animator.boneStructure_),
	scene_(animator.scene_),
	globalInverseTransform_(animator.scene_->mRootNode->mTransformation),
	animationSpeed_(animator.animationSpeed_),
	runningTime_(animator.runningTime_),
	currentAnimation_(animator.currentAnimation_),
	boneTransforms_(animator.boneTransforms_)
{
}

Animator::Animator(aiScene* scene, BoneStructure& boneStructure) :
	scene_(scene),
	boneStructure_(boneStructure)
{
	globalInverseTransform_ = scene_->mRootNode->mTransformation;
	globalInverseTransform_.Inverse();
	runningTime_ = 0.0f;
	animationSpeed_= 0.00075f;
}

bool Animator::playAnimation(std::string animationName)
{
	int numAnimations = scene_->mNumAnimations;
	aiAnimation** animations = scene_->mAnimations;
	for (int i = 0; i < numAnimations; i++)
	{
		if (std::strcmp(animations[i]->mName.C_Str(), animationName.c_str()) == 0)
		{
			currentAnimation_ = i;
			return true;
		}
	}
	currentAnimation_ = 0;
	return false;
}

void outputMatrixToFile(std::vector<aiMatrix4x4> boneTransforms)
{
	std::string output;
	for (uint32_t i = 0; i < boneTransforms.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				output += std::to_string((boneTransforms[i])[j][k]);
			}
		}
	}
	std::cout << output << std::endl;
}

std::vector<aiMatrix4x4> Animator::update(float deltaTime)
{
	boneTransforms_.resize(boneStructure_.boneInfo_.size());
	if (currentAnimation_ == -1)
	{
		aiMatrix4x4 ident = aiMatrix4x4();
		for (uint32_t i = 0; i < boneStructure_.boneInfo_.size(); i++)
		{
			boneTransforms_[i] = ident;
		}
	}
	else
	{
		aiAnimation* m_animation = scene_->mAnimations[currentAnimation_];

		runningTime_ += deltaTime * animationSpeed_;

		float TicksPerSecond = (float)(m_animation->mTicksPerSecond != 0 ?m_animation->mTicksPerSecond : 25.0f);
		float TimeInTicks = runningTime_* TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, (float)m_animation->mDuration);

		aiMatrix4x4 identity = aiMatrix4x4();
		readNodeHierarchy(AnimationTime, scene_->mRootNode, identity);

		
		for (uint32_t i = 0; i < boneStructure_.boneInfo_.size(); i++)
		{
			boneTransforms_[i] = boneStructure_.boneInfo_[i].finalTransformation;
		}
	}

	return boneTransforms_;
}

// Returns a 4x4 matrix with interpolated translation between current and next frame
aiMatrix4x4 Animator::interpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
{
	aiVector3D translation;

	if (pNodeAnim->mNumPositionKeys == 1)
	{
		translation = pNodeAnim->mPositionKeys[0].mValue;
	}
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if (time < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
		aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];


		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiVector3D& start = currentFrame.mValue;
		const aiVector3D& end = nextFrame.mValue;

		translation = (start + delta * (end - start));
	}

	aiMatrix4x4 mat;
	aiMatrix4x4::Translation(translation, mat);
	return mat;
}

// Returns a 4x4 matrix with interpolated rotation between current and next frame
aiMatrix4x4 Animator::interpolateRotation(float time, const aiNodeAnim* pNodeAnim)
{
	aiQuaternion rotation;

	if (pNodeAnim->mNumRotationKeys == 1)
	{
		rotation = pNodeAnim->mRotationKeys[0].mValue;
	}
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
		{
			if (time < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
		aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];

		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiQuaternion& start = currentFrame.mValue;
		const aiQuaternion& end = nextFrame.mValue;

		aiQuaternion::Interpolate(rotation, start, end, delta);
		rotation.Normalize();
	}

	aiMatrix4x4 mat(rotation.GetMatrix());
	return mat;
}


// Returns a 4x4 matrix with interpolated scaling between current and next frame
aiMatrix4x4 Animator::interpolateScale(float time, const aiNodeAnim* pNodeAnim)
{
	aiVector3D scale;

	if (pNodeAnim->mNumScalingKeys == 1)
	{
		scale = pNodeAnim->mScalingKeys[0].mValue;
	}
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		{
			if (time < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
		aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];

		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiVector3D& start = currentFrame.mValue;
		const aiVector3D& end = nextFrame.mValue;

		scale = (start + delta * (end - start));
	}

	aiMatrix4x4 mat;
	aiMatrix4x4::Scaling(scale, mat);
	return mat;
}


const aiNodeAnim* Animator::findNodeAnim(std::string nodeName)
{
	aiAnimation* m_animation = scene_->mAnimations[currentAnimation_];
	for (uint32_t i = 0; i < m_animation->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnim = m_animation->mChannels[i];
		if (std::string(nodeAnim->mNodeName.data) == nodeName)
		{
			return nodeAnim;
		}
	}
	return nullptr;
}

void Animator::readNodeHierarchy(float animationTime, aiNode* pNode, aiMatrix4x4 parentTransform)
{
	std::string NodeName(pNode->mName.data);

	aiMatrix4x4 NodeTransformation(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = findNodeAnim(NodeName);

	if (pNodeAnim)
	{
		// Get interpolated matrices between current and next frame
		aiMatrix4x4 matScale = interpolateScale(animationTime, pNodeAnim);
		aiMatrix4x4 matRotation = interpolateRotation(animationTime, pNodeAnim);
		aiMatrix4x4 matTranslation = interpolateTranslation(animationTime, pNodeAnim);

		NodeTransformation = matTranslation * matRotation * matScale;
	}

	aiMatrix4x4 GlobalTransformation = parentTransform * NodeTransformation;

	auto boneMapping = boneStructure_.boneMapping_;
	if (boneMapping.find(NodeName) != boneMapping.end())
	{
		uint32_t BoneIndex = boneMapping[NodeName];
		boneStructure_.boneInfo_[BoneIndex].finalTransformation = globalInverseTransform_ * GlobalTransformation * boneStructure_.boneInfo_[BoneIndex].offset;
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; i++)
	{
		readNodeHierarchy(animationTime, pNode->mChildren[i], GlobalTransformation);
	}
}
