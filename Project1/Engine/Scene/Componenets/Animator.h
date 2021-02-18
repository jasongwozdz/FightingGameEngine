#pragma once
#include "../../AnimatedGameObject.h"

class Animator
{
public:
	Animator(AnimatedGameObject& gameObject);

	void update(float deltaTime);

private:

	float runningTime;

	float animationSpeed;

	AnimatedGameObject& m_gameObject;

	std::vector<aiMatrix4x4> boneTransforms;

	aiMatrix4x4 globalInverseTransform;

	void readNodeHierarchy(float animationTime, aiNode* node, glm::mat4 parentTransform);

	void readNodeHierarchy(float animationTime, aiNode* pNode, aiMatrix4x4 parentTransform);

	const aiNodeAnim* findNodeAnim(std::string nodeName);

	aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim);

	aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim);

	aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim);
};
