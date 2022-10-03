#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include "glm/glm.hpp"
#include <array>
#include "glad/glad.h"

#define CHECK_FOR_ERROR() \
{\
	GLenum error = glGetError(); \
	_ASSERT(error == GL_NO_ERROR);\
}

template<typename VertexType>
VkVertexInputBindingDescription VertexUtil::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(VertexType);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

template VkVertexInputBindingDescription VertexUtil::getBindingDescription<Vertex>();

template VkVertexInputBindingDescription VertexUtil::getBindingDescription<NonAnimVertex>();

template VkVertexInputBindingDescription VertexUtil::getBindingDescription<UIVertex>();

template VkVertexInputBindingDescription VertexUtil::getBindingDescription<DebugVertex>();

template VkVertexInputBindingDescription VertexUtil::getBindingDescription<ParticleVertex>();

template<>
std::vector<VkVertexInputAttributeDescription> VertexUtil::getAttributeDescriptions<Vertex>()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(6);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, normal);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, boneWeights);

	attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
	attributeDescriptions[5].offset = offsetof(Vertex, boneIds);

	return attributeDescriptions;
}

template<>
void VertexUtil::setupVertexAttribPointers<NonAnimVertex>()
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NonAnimVertex), (void*)(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NonAnimVertex), (void*)(offsetof(NonAnimVertex, color)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(NonAnimVertex), (void*)(offsetof(NonAnimVertex, texCoord)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NonAnimVertex), (void*)(offsetof(NonAnimVertex, normal)));
}

template<>
void VertexUtil::setupVertexAttribPointers<Vertex>()
{
	glEnableVertexAttribArray(0);
	CHECK_FOR_ERROR();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0));
	CHECK_FOR_ERROR();

	glEnableVertexAttribArray(1);
	CHECK_FOR_ERROR();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
	CHECK_FOR_ERROR();

	glEnableVertexAttribArray(2);
	CHECK_FOR_ERROR();
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	CHECK_FOR_ERROR();

	glEnableVertexAttribArray(3);
	CHECK_FOR_ERROR();
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	CHECK_FOR_ERROR();

	glEnableVertexAttribArray(4);
	CHECK_FOR_ERROR();
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, boneWeights)));
	CHECK_FOR_ERROR();

	glEnableVertexAttribArray(5);
	CHECK_FOR_ERROR();
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneIds)));
	CHECK_FOR_ERROR();
}

template<>
std::vector<VkVertexInputAttributeDescription> VertexUtil::getAttributeDescriptions<NonAnimVertex>()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(4);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(NonAnimVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(NonAnimVertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(NonAnimVertex, texCoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(NonAnimVertex, normal);

	return attributeDescriptions;
}

template<>
std::vector<VkVertexInputAttributeDescription> VertexUtil::getAttributeDescriptions<UIVertex>()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(3);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(UIVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(UIVertex, aUV);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(UIVertex, color);

	return attributeDescriptions;
}

template<>
std::vector<VkVertexInputAttributeDescription> VertexUtil::getAttributeDescriptions<DebugVertex>()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(DebugVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(DebugVertex, color);

	return attributeDescriptions;
}

template<>
std::vector<VkVertexInputAttributeDescription> VertexUtil::getAttributeDescriptions<ParticleVertex>()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(3);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(ParticleVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(ParticleVertex, aUV);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(ParticleVertex, color);

	return attributeDescriptions;
}
