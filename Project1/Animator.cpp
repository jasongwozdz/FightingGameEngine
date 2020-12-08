#include <iostream>
#include <fstream>
#include "Animator.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

Animator::Animator(AnimatedGameObject& gameObject)
	: m_gameObject(gameObject)
{
	//globalInverseTransform = glm::make_mat4(&gameObject.m_scene->mRootNode->mTransformation.a1);
	//globalInverseTransform = glm::inverse(globalInverseTransform);

	globalInverseTransform = gameObject.m_scene->mRootNode->mTransformation;
	globalInverseTransform.Inverse();
	runningTime = 0.0f;
	animationSpeed = 0.00075f;
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

void Animator::update(float deltaTime)
{
	int currAnimationId = m_gameObject.currentAnimation;
	boneTransforms.resize(m_gameObject.m_boneInfo.size());
	if (currAnimationId == -1)
	{
		aiMatrix4x4 ident = aiMatrix4x4();
		for (uint32_t i = 0; i < m_gameObject.m_boneInfo.size(); i++)
		{
			boneTransforms[i] = ident;
		}
	}
	else
	{
		aiAnimation* m_animation = m_gameObject.m_scene->mAnimations[m_gameObject.currentAnimation];

		runningTime += deltaTime * animationSpeed;

		float TicksPerSecond = (float)(m_animation->mTicksPerSecond != 0 ?m_animation->mTicksPerSecond : 25.0f);
		float TimeInTicks = runningTime * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, (float)m_animation->mDuration);

		//glm::mat4 identity(1.0f);
		aiMatrix4x4 identity = aiMatrix4x4();
		readNodeHierarchy(AnimationTime, m_gameObject.m_scene->mRootNode, identity);

		
		for (uint32_t i = 0; i < m_gameObject.m_boneInfo.size(); i++)
		{
			boneTransforms[i] = m_gameObject.m_boneInfo[i].finalTransformation;
		}
	}

	m_gameObject.setPose(boneTransforms);
}

//// Returns a 4x4 matrix with interpolated translation between current and next frame
//glm::mat4 Animator::interpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
//{
//	glm::vec3 translation;
//
//	if (pNodeAnim->mNumPositionKeys == 1)
//	{
//		translation = glm::make_vec3(&pNodeAnim->mPositionKeys[0].mValue.x);
//	}
//	else
//	{
//		uint32_t frameIndex = 0;
//		for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
//		{
//			if (time < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
//			{
//				frameIndex = i;
//				break;
//			}
//		}
//
//		aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
//		aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];
//
//		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
//
//		const aiVector3D& start = currentFrame.mValue;
//		const aiVector3D& end = nextFrame.mValue;
//
//		aiVector3D trans = (start + delta * (end - start));
//		translation = glm::make_vec3(&trans.x);
//	}
//
//	glm::mat4 mat(1.0f);
//	mat = glm::translate(mat, translation);
//	mat = glm::transpose(mat);
//	return mat;
//}
//
//// Returns a 4x4 matrix with interpolated rotation between current and next frame
//glm::mat4 Animator::interpolateRotation(float time, const aiNodeAnim* pNodeAnim)
//{
//	aiQuaternion rotation;
//
//	if (pNodeAnim->mNumRotationKeys == 1)
//	{
//		rotation = pNodeAnim->mRotationKeys[0].mValue;
//	}
//	else
//	{
//		uint32_t frameIndex = 0;
//		for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
//		{
//			if (time < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
//			{
//				frameIndex = i;
//				break;
//			}
//		}
//
//		aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
//		aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];
//
//		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
//
//		const aiQuaternion& start = currentFrame.mValue;
//		const aiQuaternion& end = nextFrame.mValue;
//		
//		aiQuaternion::Interpolate(rotation, start, end, delta);
//		rotation.Normalize();
//	}
//
//	aiMatrix4x4 mat(rotation.GetMatrix());
//	
//	glm::mat4 rot = glm::make_mat4(&mat.a1);
//	
//	return rot;
//}
//
//// Returns a 4x4 matrix with interpolated scaling between current and next frame
//glm::mat4 Animator::interpolateScale(float time, const aiNodeAnim* pNodeAnim)
//{
//	glm::vec3 scale;
//
//	if (pNodeAnim->mNumScalingKeys == 1)
//	{
//		scale = glm::make_vec3(&pNodeAnim->mScalingKeys[0].mValue.x);
//	}
//	else
//	{
//		uint32_t frameIndex = 0;
//		for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
//		{
//			if (time < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
//			{
//				frameIndex = i;
//				break;
//			}
//		}
//
//		aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
//		aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];
//
//		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
//
//		const aiVector3D& start = currentFrame.mValue;
//		const aiVector3D& end = nextFrame.mValue;
//
//		aiVector3D s = (start + delta * (end - start));
//		scale = glm::make_vec3(&s.x);
//	}
//
//	glm::mat4 mat(1.0f);
//	mat = glm::scale(mat, scale);
//	return mat;
//}


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
	aiAnimation* m_animation = m_gameObject.m_scene->mAnimations[m_gameObject.currentAnimation];
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

void Animator::readNodeHierarchy(float animationTime, aiNode* pNode, glm::mat4 parentTransform)
{
	//std::string nodeName(pNode->mName.data);

	////glm::mat4 nodeTransformation = glm::make_mat4(&pNode->mTransformation.a1);

	//aiMatrix4x4 nodeTransformation(pNode->mTransformation);

	//const aiNodeAnim* pNodeAnim = findNodeAnim(nodeName);

	//if (pNodeAnim)
	//{
	//	//Get interpolated matrices between current and next frame
	//	glm::mat4 matScale = interpolateScale(animationTime, pNodeAnim);
	//	glm::mat4 matRotation = interpolateRotation(animationTime, pNodeAnim);
	//	glm::mat4 matTranslation = interpolateTranslation(animationTime, pNodeAnim);
	//	
	//	nodeTransformation = matTranslation * matRotation * matScale;
	//}

	//glm::mat4 GlobalTransformation = parentTransform * nodeTransformation;

	//auto boneMapping = m_gameObject.m_boneMapping;
	//if (boneMapping.find(nodeName) != boneMapping.end())
	//{
	//	uint32_t BoneIndex = boneMapping[nodeName];
	//	m_gameObject.m_boneInfo[BoneIndex].finalTransformation = globalInverseTransform * GlobalTransformation * m_gameObject.m_boneInfo[BoneIndex].offset;
	//}

	//for (uint32_t i = 0; i < pNode->mNumChildren; i++)
	//{
	//	readNodeHierarchy(animationTime, pNode->mChildren[i], GlobalTransformation);
	//}
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

	auto boneMapping = m_gameObject.m_boneMapping;
	if (boneMapping.find(NodeName) != boneMapping.end())
	{
		uint32_t BoneIndex = boneMapping[NodeName];
		m_gameObject.m_boneInfo[BoneIndex].finalTransformation = globalInverseTransform * GlobalTransformation * m_gameObject.m_boneInfo[BoneIndex].offset;
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; i++)
	{
		readNodeHierarchy(animationTime, pNode->mChildren[i], GlobalTransformation);
	}
}
