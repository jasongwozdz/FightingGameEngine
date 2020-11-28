#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include "GameObject.h"

GameObject::GameObject(std::string modelPath, std::string texturePath, glm::vec3 pos)
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
		texturePath
	);
	position = pos;
}

GameObject::GameObject(Mesh* mesh, glm::vec3 pos)
{
	p_mesh = mesh;
	position = pos;
}

void GameObject::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	p_mesh->m_ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
}

Mesh* GameObject::getMeshPtr()
{
	return p_mesh;
}
