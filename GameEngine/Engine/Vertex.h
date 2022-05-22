#pragma once
#include "Vertex.h"

#include <array>
#include <vector>

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"


struct Vertex {

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 boneWeights = {0.0, 0.0, 0.0, 0.0};
	glm::ivec4 boneIds = {0, 0, 0, 0};

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal && boneWeights == other.boneWeights && boneIds == other.boneIds;
	}
};

struct NonAnimVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const NonAnimVertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
	}
};

struct UIVertex
{
	glm::vec2 pos;
	glm::vec2 aUV;
	glm::vec4 color;
};

struct DebugVertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct ParticleVertex
{
	glm::vec3 pos;
	glm::vec2 aUV;
	glm::vec4 color;
};

namespace VertexUtil
{
	template<typename VertexType>
	VkVertexInputBindingDescription getBindingDescription();

	template<typename VertexType>
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
}


