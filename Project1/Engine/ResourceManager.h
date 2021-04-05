#pragma once
#include "Singleton.h"
#include <vulkan/vulkan.h>
#include <map>
#include <iostream>
#include <vector>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include "Vertex.h"
#include "Scene/Componenets/Animator.h"


#define MAX_BONES_PER_VERTEX 4
#define MAX_BONES 64

class BoneStructure;

struct VertexBoneInfo
{
	glm::uvec4 ids = {0, 0, 0, 0}; //bone indices that affect this vertex
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

struct BoneInfo
{
	aiMatrix4x4 offset; //transformation from mesh space to local space of current bone
	aiMatrix4x4 finalTransformation;
};

enum ReturnTypes
{
	Texture, 
	Model,
	Bone
};

struct ReturnVals {
	virtual ReturnTypes getType() = 0;

	virtual ~ReturnVals() = default;
};

struct TextureReturnVals : ReturnVals
{
	virtual ReturnTypes getType()
	{
		return Texture;
	}

	TextureReturnVals(
		unsigned char* pixels,
		int textureWidth,
		int textureHeight,
		int textureChannels
	) : textureWidth(textureWidth),textureHeight(textureHeight),textureChannels(textureChannels)
	{
		this->pixels.resize(textureWidth*textureHeight*4);
		memcpy(this->pixels.data(), pixels, textureWidth*textureHeight*4);
	}

	TextureReturnVals(const TextureReturnVals& vals)
	{
		pixels = vals.pixels;
		textureWidth = vals.textureWidth;
		textureHeight = vals.textureHeight;
		textureChannels = vals.textureChannels;
	}

	~TextureReturnVals() {};

	std::vector<unsigned char> pixels;
	int textureWidth;
	int textureHeight;
	int textureChannels;
};

struct ModelReturnVals : ReturnVals{
	virtual ReturnTypes getType()
	{
		return Model;
	}

	ModelReturnVals()
	{}

	ModelReturnVals(const ModelReturnVals& vals)
	{
		vertices = vals.vertices;
		indices = vals.indices;
	}

	~ModelReturnVals() {
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct AnimationReturnVals : ReturnVals
{
	virtual ReturnTypes getType()
	{
		return Bone;
	}

	~AnimationReturnVals() {};
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	BoneStructure* boneStructure;
	std::vector<AnimationClip> animations;
};



class ResourceManager : Singleton<ResourceManager>
{
public:
	~ResourceManager();

	static glm::mat4 aiMatToGlmMat(aiMatrix4x4& a);
	
	std::map<std::string, uintptr_t> m_resourceRegistry;

	ModelReturnVals& loadObjFile(std::string& filePath);

	TextureReturnVals& loadTextureFile(std::string& filePath);

	AnimationReturnVals& loadAnimationFile(std::string& filePath);

	BoneStructure& getBoneStructure(int index); //get Bone structure by index

	void freeResource(std::string filePath);

	void freeAllResources();

	static ResourceManager& getSingleton();

	static ResourceManager* getSingletonPtr();

	Assimp::Importer importer; //importer owns scene object so need to keep a copy of it

private:

	std::vector<BoneStructure> boneStructures_;

	inline aiBone* findBoneName(std::string boneName, aiBone** bones, int numBones)
	{
		for (int i = 0; i < numBones; i++)
		{
			if (strcmp(bones[i]->mName.C_Str(), boneName.c_str()) == 0)
			{
				return bones[i];
			}
		}
		return nullptr;
	}

	aiBone* findRootBone(aiNode* node, aiMesh* mesh);

	void populateBoneStructure(aiNode* root, aiMesh* mesh, BoneStructure& boneStructure);

	void recursivePopulateBoneStructure(aiNode* node, aiMesh* mesh, BoneStructure& boneStructure);

	void recursivePopulateBoneStructure(aiNode* node, aiMesh* mesh, BoneStructure& boneStructure, int parentIndex);

	bool populateAnimationClips(aiAnimation** animations, int numAnimations, BoneStructure& bones, std::vector<AnimationClip>& animationClips);

	bool populateAnimationClip(AnimationClip& sample, aiNodeAnim** animationNodes, int numChannels, BoneStructure& bones);
};
