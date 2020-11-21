#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include "GameObject.h"
#include <glm/glm.hpp>

GameObject::GameObject(std::string modelPath, std::string texturePath, Position pos)
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
	p_mesh->m_ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
	pos = position;
}

void GameObject::setPosition(Position newPosition)
{
	position = newPosition;
}

Mesh* GameObject::getMeshPtr()
{
	return p_mesh;
}
