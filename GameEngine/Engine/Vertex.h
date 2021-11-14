#pragma once
#include "Vertex.h"
#include "glm/glm.hpp"
#include <array>

class VkVertexInputBindingDescription;
class VkVertexInputAttributeDescription;

struct Vertex {

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 boneWeights = {0.0, 0.0, 0.0, 0.0};
	glm::ivec4 boneIds = {0, 0, 0, 0};

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions();

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

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

