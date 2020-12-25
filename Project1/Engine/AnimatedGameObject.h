#pragma once
#include "GameObject.h"
#include "assimp/scene.h"
#include <Vector>
class AnimatedGameObject :
	public GameObject
{
public:
	AnimatedGameObject(std::vector<Vertex> vertices, 
		std::vector<uint32_t> indices, 
		glm::vec3 pos, 
		VkDescriptorSetLayout layout,
		uint32_t currentAnim,
		const aiScene* scene,
		std::map<std::string, uint32_t> bonemapping,
		std::vector<BoneInfo> boneInfo);

	~AnimatedGameObject();

	void setPose(std::vector<aiMatrix4x4> pose);

	void setAnimation(int animationId);

	int currentAnimation;
	const aiScene* m_scene;
	std::map<std::string, uint32_t> m_boneMapping;
	std::vector<BoneInfo> m_boneInfo;
	aiBone* m_rootBone;
};

