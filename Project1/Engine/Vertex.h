#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include "glm/glm.hpp"
#include <array>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 boneWeights = {0.0, 0.0, 0.0, 0.0};
	glm::ivec4 boneIds = {0, 0, 0, 0};

	//Vertex() {};

	//Vertex(glm::vec3 pos, glm::vec3 color) :
	//	pos(pos), color(color){}

	//Vertex(	glm::vec3 pos,
	//	glm::vec3 color,
	//	glm::vec2 texCoord,
	//	glm::vec3 normal
	//) : pos(pos), color(color), texCoord(texCoord), normal(normal)
	//{
	//	glm::vec4 boneWeights = {0.0, 0.0, 0.0, 0.0};
	//	glm::ivec4 boneIds = {0, 0, 0, 0};
	//}
	//
	//Vertex(	glm::vec3 pos,
	//	glm::vec3 color,
	//	glm::vec2 texCoord,
	//	glm::vec3 normal,
	//	glm::vec4 boneWeights,
	//	glm::ivec4 boneIds
	//) : pos(pos), color(color), texCoord(texCoord), normal(normal),boneWeights(boneWeights),boneIds(boneIds){}

	//Vertex(const Vertex& v)
	//{
	//	pos = v.pos;
	//	color = v.color;
	//	texCoord = v.texCoord;
	//	normal = v.normal;
	//	boneWeights = v.boneWeights;
	//	boneIds = v.boneIds;
	//}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

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


	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal && boneWeights == other.boneWeights && boneIds == other.boneIds;
	}
};
