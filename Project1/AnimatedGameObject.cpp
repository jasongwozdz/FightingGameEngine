#include "AnimatedGameObject.h"
#include "AnimatedMesh.h"
#include <glm/gtc/type_ptr.hpp>

AnimatedGameObject::AnimatedGameObject(std::vector<Vertex> vertices,
	std::vector<uint32_t> indices,
	glm::vec3 pos,
	VkDescriptorSetLayout layout,
	uint32_t currentAnim,
	const aiScene* scene,
	std::map<std::string, uint32_t> boneMapping,
	std::vector<BoneInfo> boneInfo) :
	m_boneMapping(boneMapping),
	m_boneInfo(boneInfo),
	m_scene(scene)
{
	std::string texturePath("./Textures/viking_room.png");
	Renderer& renderer = Renderer::getSingleton();
	p_mesh = new AnimatedMesh(vertices,
		indices,
		renderer.getCommandBuffers(),
		renderer.getLogicalDevice(),
		renderer.getSwapChainImages(),
		renderer.getSwapChainExtent(),
		renderer.getPhysicalDevice(),
		renderer.getCommandPool(),
		renderer.getGraphicsQueue(),
		texturePath,
		layout
	);
};

AnimatedGameObject::~AnimatedGameObject()
{
	delete p_mesh;
}

void AnimatedGameObject::setPose(std::vector<aiMatrix4x4> pose)
{
	AnimatedMesh* mesh = reinterpret_cast<AnimatedMesh*>(p_mesh);
	mesh->animUbo = Anim_UBO(p_mesh->m_ubo);
	for (int i = 0; i < pose.size(); ++i)
	{
		mesh->animUbo.bones[i] =  glm::transpose(glm::make_mat4(&pose[i].a1));
		//mesh->animUbo.bones[i] = glm::transpose(pose[i]);
		//mesh->animUbo.bones[i] = glm::mat4(1.0f);
	}
	for (int i = MAX_BONES - (MAX_BONES - pose.size()); i < MAX_BONES; i++)
	{
		mesh->animUbo.bones[i] = glm::mat4(1.0f);
	}
}

void AnimatedGameObject::setAnimation(int animationId)
{
	//assert(animationId < m_scene->mNumAnimations);
	currentAnimation = animationId;
	return;
}

