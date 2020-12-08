#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include "GameObject.h"

GameObject::GameObject(){};

GameObject::GameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, VkDescriptorSetLayout layout)
{
	ModelReturnVals vals = ResourceManager::getSingleton().loadObjFile(modelPath);
	Renderer& renderer = Renderer::getSingleton();
	p_mesh = new TexturedMesh(vals.vertices,
		vals.indices,
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
	position = pos;
}

GameObject::GameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, VkDescriptorSetLayout layout)
{
	Renderer& renderer = Renderer::getSingleton();

	p_mesh = new PrimitiveMesh(vertices, indices, renderer.getCommandBuffers(), renderer.getLogicalDevice(), renderer.getSwapChainImages(), renderer.getSwapChainExtent(), renderer.getPhysicalDevice(), renderer.getCommandPool(), renderer.getGraphicsQueue(), layout);
	position = pos;

}

GameObject::~GameObject()
{
	delete p_mesh;
}

void GameObject::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	p_mesh->m_ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
	p_mesh->m_ubo.model = glm::scale(p_mesh->m_ubo.model, scale);
}

void GameObject::setScale(glm::vec3 newScale)
{
	scale = newScale;
	p_mesh->m_ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
	p_mesh->m_ubo.model = glm::scale(p_mesh->m_ubo.model, scale);
}

Mesh* GameObject::getMeshPtr()
{
	return p_mesh;
}
