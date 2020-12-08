#pragma once
#include "Singleton.h"
#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include "Vertex.h"

#define MAX_BONES_PER_VERTEX 4
#define MAX_BONES 64

struct JointTransform
{
	glm::vec3 position;
	glm::quat quat;
};

struct KeyFrame
{
	std::vector<JointTransform> transforms;
	float timeStamp;
};

struct TextureReturnVals
{
	unsigned char* pixels;
	int textureWidth;
	int textureHeight;
	int textureChannels;
};

struct ModelReturnVals
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<JointTransform> joints;
};

struct VertexBoneInfo
{
	glm::uvec4 ids = {0, 0, 0, 0}; //bone indices that effect this vertex
	glm::vec4 weights = {0.0, 0.0, 0.0, 0.0}; //weights of each bones transformation on this vertex

	void add(uint32_t id, float weight)
	{
		for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
		{
			if (weights[i] == 0)
			{
				weights[i] = weight;
				ids[i] = id;
				return;
			}

		}
	}
};

//struct BoneInfo
//{
//	glm::mat4 offset; //transformation from mesh space to local space of current bone
//	glm::mat4 finalTransformation;
//};

struct BoneInfo
{
	aiMatrix4x4 offset; //transformation from mesh space to local space of current bone
	aiMatrix4x4 finalTransformation;
};

struct AnimationReturnVals
{
	std::map<std::string, uint32_t> boneMapping;
	std::vector<BoneInfo> boneInfo;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	const aiScene* scene;
};


class ResourceManager : Singleton<ResourceManager>
{
public:

	static glm::mat4 aiMatToGlmMat(aiMatrix4x4& a);
	
	std::map<std::string, uintptr_t> m_resourceRegistry;

	ModelReturnVals& loadObjFile(std::string& filePath);
	
	TextureReturnVals& loadTextureFile(std::string& filePath);

	AnimationReturnVals& loadAnimationFile(std::string& filePath);

	void freeResource(std::string filePath);

	void freeAllResources();

	static ResourceManager& getSingleton();

	static ResourceManager* getSingletonPtr();

	Assimp::Importer importer; //importer owns scene object so need to keep a copy of it

};

