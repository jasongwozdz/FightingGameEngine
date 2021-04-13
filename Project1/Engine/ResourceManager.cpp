#define GLM_ENABLE_EXPERIMENTAL
#include "ResourceManager.h"
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <glm/gtx/hash.hpp>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <assert.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define MAX_BONES 64

/*
should write a load texture file that loads in textures pixel data given a path to it.

Should also handle referential integrity between game objects

*/

ResourceManager::~ResourceManager()
{
	freeAllResources();
}

glm::mat4 ResourceManager::aiMatToGlmMat(aiMatrix4x4& a)
{
	glm::mat4 mat;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; ++j)
		{
			mat[i][j] = a[j][i];
		}
	}
	return mat;
}


template<> ResourceManager* Singleton<ResourceManager>::msSingleton = 0;

ResourceManager* ResourceManager::getSingletonPtr()
{
	return msSingleton;
}

ResourceManager& ResourceManager::getSingleton()
{
	assert(msSingleton);  return (*msSingleton);
}


namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

ModelReturnVals& ResourceManager::loadObjFile(std::string& filePath) {
	auto find = m_resourceRegistry.find(filePath);
	ModelReturnVals* vals;
	if (find == m_resourceRegistry.end())
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				
				indices.push_back(uniqueVertices[vertex]);
			}
		}
		vals = new ModelReturnVals();
		vals->vertices = vertices;
		vals->indices = indices;
		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(vals);
	}
	else
	{
		vals = reinterpret_cast<ModelReturnVals*>(m_resourceRegistry[filePath]);
	}
	return *vals;
}

bool ResourceManager::populateAnimationClip(AnimationClip& sample, aiNodeAnim** animationNodes, int numChannels, BoneStructure& bones)
{
	sample.positions_.resize(bones.boneInfo_.size());
	sample.rotations_.resize(bones.boneInfo_.size());
	sample.scale_.resize(bones.boneInfo_.size());
	for (int i = 0; i < numChannels; i++)
	{
		aiNodeAnim* currentNode = animationNodes[i];
		int boneIndex = bones.findBoneByBoneName(currentNode->mNodeName.C_Str());
		bones.boneInfo_[boneIndex].animated_ = true;
		if (boneIndex == -1)
		{
			std::cout << "ERROR: could not populate animation sample" << std::endl;
			return false;
		}
		
		//populate positions
		for (int j = 0; j < currentNode->mNumPositionKeys; j++)
		{
			glm::vec3 pos = glm::make_vec3(&currentNode->mPositionKeys[j].mValue.x);
			float time = currentNode->mPositionKeys[j].mTime;
			sample.positions_[boneIndex].push_back({ time, pos });
		}

		//populate rotations 
		for (int j = 0; j < currentNode->mNumRotationKeys; j++)
		{
			glm::quat rot;
			rot.x = currentNode->mRotationKeys[j].mValue.x;
			rot.y = currentNode->mRotationKeys[j].mValue.y;
			rot.z = currentNode->mRotationKeys[j].mValue.z;
			rot.w = currentNode->mRotationKeys[j].mValue.w;
			//{ currentNode->mRotationKeys[j].mValue.x, currentNode->mRotationKeys[j].mValue.y, currentNode->mRotationKeys[j].mValue.z, currentNode->mRotationKeys[j].mValue.w };
			float time = currentNode->mRotationKeys[j].mTime;
			sample.rotations_[boneIndex].push_back({ time, rot });
		}

		//populate scale 
		for (int j = 0; j < currentNode->mNumScalingKeys; j++)
		{
			float scale = currentNode->mScalingKeys[j].mValue.x;
			float time = currentNode->mRotationKeys[j].mTime;
			sample.scale_[boneIndex].push_back({ time, scale });
		}

	}
	return true;
}


 bool ResourceManager::populateAnimationClips(aiAnimation** animations, int numAnimations, BoneStructure& bones, std::vector<AnimationClip>& animationClips)
{
	for (int i = 0; i < numAnimations; i++)
	{
		AnimationClip a;
		a.name_ = animations[i]->mName.C_Str();
		a.duration_ = animations[i]->mDuration;
		a.framesPerSecond_ = animations[i]->mTicksPerSecond;
		if (!populateAnimationClip(a, animations[i]->mChannels, animations[i]->mNumChannels, bones))
		{
			std::cout << "ERROR: failed to populate animation clip" << std::endl;
			return false;
		}
		else
		{
			animationClips.push_back(a);
		}
	}
	return true;
}

void ResourceManager::recursivePopulateBoneStructure(aiNode* node, aiMesh* mesh, BoneStructure& boneStructure, int parentIndex)
{
	Joint j = { glm::transpose(glm::make_mat4(&node->mTransformation.a1)), node->mName.C_Str(), parentIndex };
	boneStructure.boneInfo_.push_back(j);
	int currentIndex = boneStructure.boneInfo_.size()-1;
	aiNode** children = node->mChildren;
	for (int i = 0; i < node->mNumChildren; i++)
	{
		recursivePopulateBoneStructure(children[i], mesh, boneStructure, currentIndex);
	}
	
}

void ResourceManager::recursivePopulateBoneStructure(aiNode* node, aiMesh* mesh, BoneStructure& boneStructure)
{
	aiNode** children = node->mChildren;
	for (int i = 0; i < node->mNumChildren; i++)
	{
		std::string childName = children[i]->mName.C_Str();
		
		Joint j = {glm::transpose(glm::make_mat4(&children[i]->mTransformation.a1)), children[i]->mName.C_Str(), boneStructure.findBoneByBoneName(node->mName.C_Str())};

		aiBone* childBone = findBoneName(childName, mesh->mBones, mesh->mNumBones);

		if (childBone)
		{
			j.invBindPose_ = glm::transpose(glm::make_mat4(&childBone->mOffsetMatrix.a1));
		}

		boneStructure.boneInfo_.push_back(j);

		recursivePopulateBoneStructure(children[i], mesh, boneStructure);
	}

}

aiBone* ResourceManager::findRootBone(aiNode* node, aiMesh* mesh)
{
	aiBone* bone = findBoneName(node->mName.C_Str(), mesh->mBones, mesh->mNumBones);
	if(bone)
	{
		aiBone* parent = findBoneName(node->mParent->mName.C_Str(), mesh->mBones, mesh->mNumBones);
		if (parent)
		{
			return bone;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		aiBone* ret = findRootBone(node->mChildren[i], mesh);
		if (ret)
		{
			return ret;
		}
	}
	return nullptr;
}

void ResourceManager::populateBoneStructure(aiNode* root, aiMesh* mesh, BoneStructure& boneStructure)
{
	boneStructure.globalInverseTransform_ = glm::inverse(glm::transpose(glm::make_mat4(&root->mTransformation.a1)));
	//boneStructure.boneInfo_.push_back({ glm::transpose(glm::make_mat4(&root->mTransformation.a1)), root->mName.C_Str(), -1});
	//recursivePopulateBoneStructure(root, mesh, boneStructure);
	recursivePopulateBoneStructure(root, mesh, boneStructure, 0);
}

AnimationReturnVals& ResourceManager::loadAnimationFile(std::string& filePath)
{
	auto find = m_resourceRegistry.find(filePath);
	AnimationReturnVals* vals;
	if (find == m_resourceRegistry.end())
	{
		std::map<std::string, uint32_t> boneMapping;
		std::vector<BoneInfo> boneInfo;
		std::vector<VertexBoneInfo> vertexBoneInfo;
		std::vector<AnimationClip> animationClips;
		BoneStructure* boneStructure = new BoneStructure(0);

		const aiScene* scene = importer.ReadFile(filePath
			.c_str(), 0);

		uint32_t vertexCount(0);		
		for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
			vertexCount += scene->mMeshes[m]->mNumVertices;
		};
		vertexBoneInfo.resize(vertexCount);
		int numBones(0);

		int vertexOffset(0);
		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* pAiMesh = scene->mMeshes[i];

			populateBoneStructure(scene->mRootNode, pAiMesh, *boneStructure);
			populateAnimationClips(scene->mAnimations, scene->mNumAnimations, *boneStructure, animationClips);
			for (uint32_t j = 0; j < pAiMesh->mNumBones; ++j)
			{
				aiBone* currBone = pAiMesh->mBones[j];
				assert(pAiMesh->mNumBones <= MAX_BONES);
				 
				const uint32_t numWeights = currBone->mNumWeights;
				//add boneIDs/weights for each vertex 
				std::string boneNameS = std::string(currBone->mName.C_Str());
				int boneIndex = boneStructure->findBoneByBoneName(boneNameS);
				boneStructure->boneInfo_[boneIndex].offset_ = glm::transpose(glm::make_mat4(&currBone->mOffsetMatrix.a1));
				for (uint32_t k = 0; k < numWeights; ++k)
				{
					uint32_t vertexIdx = currBone->mWeights[k].mVertexId + vertexOffset;
					vertexBoneInfo[vertexIdx].add(boneIndex, currBone->mWeights[k].mWeight);//bone index, bone weight
				}
			}
			vertexOffset += pAiMesh->mNumVertices;
		}

		//generate Vertex and index vectors
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		int vertexBase(0);
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* pAiMesh = scene->mMeshes[i];
			for (uint32_t j = 0; j < pAiMesh->mNumVertices; ++j)
			{
				Vertex vertex{};
				vertex.pos = glm::make_vec3(&pAiMesh->mVertices[j].x);
				vertex.normal = glm::make_vec3(&pAiMesh->mNormals[j].x);
				vertex.texCoord = glm::make_vec2(&pAiMesh->mTextureCoords[0][j].x);
				vertex.color = { 255, 0, 0 };
				
				for (uint32_t k = 0; k < MAX_BONES_PER_VERTEX; ++k)
				{
					vertex.boneIds[k] = vertexBoneInfo[vertexBase + j].ids[k];
					vertex.boneWeights[k] = vertexBoneInfo[vertexBase + j].weights[k];
				}

				vertices.push_back(vertex);
			}

			vertexBase += pAiMesh->mNumVertices;

			uint32_t indexBase = static_cast<uint32_t>(indices.size());
			for (uint32_t j = 0; j < pAiMesh->mNumFaces; ++j)
			{
				for (uint32_t f = 0; f < 3; ++f)
				{
					indices.push_back(pAiMesh->mFaces[j].mIndices[f] + indexBase);
				}
			}
		}

		vals = new AnimationReturnVals();
		vals->vertices = vertices;
		vals->indices = indices;
		vals->boneStructure = boneStructure;
		vals->animations = animationClips;
		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(vals);
	}
	else
	{
		vals = reinterpret_cast<AnimationReturnVals*>(m_resourceRegistry[filePath]);
	}
	
	return *vals;
}

TextureReturnVals& ResourceManager::loadTextureFile(std::string& filePath)
{
	TextureReturnVals* returnVals;
	//Look in resourceRegistry for texture.  If its not found, allocate resources and then add to registry.
	//If it is found grab pointer from registry
	auto find = m_resourceRegistry.find(filePath);
	if (find == m_resourceRegistry.end())
	{
		int texWidth, texHeight, texChannels;
		unsigned char* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cout << "invalid texture path" << std::endl;
			assert(1 == 0);
		}

		returnVals = new TextureReturnVals(pixels, texWidth, texHeight, texChannels);

		stbi_image_free(pixels);

		m_resourceRegistry[filePath] = reinterpret_cast<uintptr_t>(returnVals);
	}
	else
	{
		returnVals = reinterpret_cast<TextureReturnVals*>(find->second);
	}

	return *returnVals;
}

void ResourceManager::freeResource(std::string filePath)
{
	auto find = m_resourceRegistry.find(filePath);
	if (find == m_resourceRegistry.end())
	{
		std::cout << "Resource not in registry" << std::endl;
		assert(1 == 0);
	}
	else
	{
		delete reinterpret_cast<void*>(m_resourceRegistry[filePath]);
	}
}

void ResourceManager::freeAllResources() 
{
	for (auto const& resource : m_resourceRegistry)
	{
		//delete reinterpret_cast<ReturnVals*>(resource.second);
		ReturnVals* pResource = reinterpret_cast<ReturnVals*>(resource.second);
		switch (pResource->getType())
		{
			case Texture: 
			{
				delete dynamic_cast<TextureReturnVals*>(pResource);
				break;
			}
			case Model:
			{
				delete dynamic_cast<ModelReturnVals*>(pResource);
				break;
			}
			case Bone:
			{
				delete dynamic_cast<AnimationReturnVals*>(pResource);
				break;
			}
		}
	}
}
