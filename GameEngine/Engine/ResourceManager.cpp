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
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Renderer/Asset/Asset.h"

extern const int MAX_BONES;

/*
should write a load texture file that loads in textures pixel data given a path to it.

Should also handle referential integrity between game objects

*/

ResourceManager::~ResourceManager()
{
	freeAllResources();
	assetMap_.clear();
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
	template<> struct hash<NonAnimVertex> {
		size_t operator()(NonAnimVertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

ModelReturnVals& ResourceManager::loadObjFile(const std::string& filePath) {
	auto find = resourceRegistry_.find(filePath);
	ModelReturnVals* vals;
	if (find == resourceRegistry_.end())
	{
		std::vector<NonAnimVertex> vertices;
		std::vector<uint32_t> indices;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<NonAnimVertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				NonAnimVertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				if (index.texcoord_index != -1)//obj might not have tex coords
				{
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (index.normal_index != -1) //obj might not have a normals
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				
				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
				};

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
		resourceRegistry_[filePath] = reinterpret_cast<uintptr_t>(vals);
	}
	else
	{
		vals = reinterpret_cast<ModelReturnVals*>(resourceRegistry_[filePath]);
	}
	return *vals;
}

bool ResourceManager::populateAnimationClip(AnimationClip& sample, aiNodeAnim** animationNodes, int numChannels, BoneStructure& bones)
{
	sample.positions_.resize(bones.boneInfo_.size());
	sample.rotations_.resize(bones.boneInfo_.size());
	sample.scale_.resize(bones.boneInfo_.size());

	//find first channel that has keyframes and extract the timing for each keyframe
	for (int i = 0; i < numChannels; i++)
	{
		aiNodeAnim* currentNode = animationNodes[i];
		if (currentNode->mNumPositionKeys > 0)
		{
			for (int j = 0; j < currentNode->mNumPositionKeys; j++)
			{
				float time = currentNode->mPositionKeys[j].mTime/sample.framesPerSecond_;
				sample.times_.push_back(time);
			}
			break;
		}
	}

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
			sample.positions_[boneIndex].push_back(pos);
		}

		//populate rotations 
		for (int j = 0; j < currentNode->mNumRotationKeys; j++)
		{
			glm::quat rot;
			rot.x = currentNode->mRotationKeys[j].mValue.x;
			rot.y = currentNode->mRotationKeys[j].mValue.y;
			rot.z = currentNode->mRotationKeys[j].mValue.z;
			rot.w = currentNode->mRotationKeys[j].mValue.w;
			sample.rotations_[boneIndex].push_back(rot);
		}

		//populate scale 
		for (int j = 0; j < currentNode->mNumScalingKeys; j++)
		{
			float scale = currentNode->mScalingKeys[j].mValue.x;
			sample.scale_[boneIndex].push_back(scale);
		}

	}

	for (auto positions : sample.positions_)
	{
		if(positions.size() > 0)
		{
			sample.frameCount_ = sample.positions_[2].size();
		}
	}
	return true;
}


 bool ResourceManager::populateAnimationClips(aiAnimation** animations, int numAnimations, BoneStructure& bones, std::vector<AnimationClip>& animationClips)
{
	 static unsigned int currAnim = 0; //used to populate an animation name incase its empty
	for (int i = 0; i < numAnimations; i++)
	{
		AnimationClip a;
		a.name_ = animations[i]->mName.C_Str();
		if (a.name_.size() == 0)
		{
			std::ostringstream s;
			s << "Animation_" << currAnim;
			a.name_ = s.str();
		}
		a.duration_ = animations[i]->mDuration;
		a.framesPerSecond_ = animations[i]->mTicksPerSecond;
		a.durationInSeconds_ = (1 / a.framesPerSecond_) * a.duration_;
		if (!populateAnimationClip(a, animations[i]->mChannels, animations[i]->mNumChannels, bones))
		{
			std::cout << "ERROR: failed to populate animation clip" << std::endl;
			return false;
		}
		else
		{
			animationClips.push_back(a);
		}
		a.frameCount_ = a.positions_[a.positions_.size() - 1].size();
	}
	return true;
}

void ResourceManager::recursivePopulateBoneStructure(aiNode* node, aiMesh* mesh, BoneStructure& boneStructure, int parentIndex)
{
	Joint j;
	j.invBindPose_ = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
	j.parent_ = parentIndex;
	j.name_ = node->mName.C_Str();
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
		
		Joint j;
		j.invBindPose_ = glm::transpose(glm::make_mat4(&children[i]->mTransformation.a1));
		j.parent_ = boneStructure.findBoneByBoneName(node->mName.C_Str());
		j.name_ = children[i]->mName.C_Str();

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
	recursivePopulateBoneStructure(root, mesh, boneStructure, 0);
}

AnimationReturnVals ResourceManager::loadAnimationFile(const std::string& filePath)
{
	auto find = resourceRegistry_.find(filePath);
	if (find == resourceRegistry_.end())
	{
		std::map<std::string, uint32_t> boneMapping;
		std::vector<BoneInfo> boneInfo;
		std::vector<VertexBoneInfo> vertexBoneInfo;
		std::vector<AnimationClip> animationClips;
		BoneStructure boneStructure(0);//initialize boneStructure

		const aiScene* scene = importer.ReadFile(filePath
			.c_str(), 0);
		if (!scene)
		{
			AnimationReturnVals vals;
			vals.succesful = false;
			return vals;
		}

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

			populateBoneStructure(scene->mRootNode, pAiMesh, boneStructure);
			populateAnimationClips(scene->mAnimations, scene->mNumAnimations, boneStructure, animationClips);
			for (uint32_t j = 0; j < pAiMesh->mNumBones; ++j)
			{
				aiBone* currBone = pAiMesh->mBones[j];
				assert(pAiMesh->mNumBones <= MAX_BONES);
				 
				const uint32_t numWeights = currBone->mNumWeights;
				//add boneIDs/weights for each vertex 
				std::string boneNameS = std::string(currBone->mName.C_Str());
				int boneIndex = boneStructure.findBoneByBoneName(boneNameS);
				if (boneIndex == -1)
				{
					std::cout << "ResourceManager::loadAnimationFile ERROR: couldn't find bone" << std::endl;
					continue;
				}
				boneStructure.boneInfo_[boneIndex].offset_ = glm::transpose(glm::make_mat4(&currBone->mOffsetMatrix.a1));
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
			bool hasColor = false;
			for (uint32_t j = 0; j < pAiMesh->mNumVertices; ++j)
			{
				Vertex vertex{};
				vertex.pos = glm::make_vec3(&pAiMesh->mVertices[j].x);
				vertex.normal = glm::make_vec3(&pAiMesh->mNormals[j].x);
				vertex.texCoord = glm::make_vec2(&pAiMesh->mTextureCoords[0][j].x);
				if (hasColor)
				{
					vertex.color = glm::make_vec3(&pAiMesh->mColors[0][j].r);
				}
				else
				{
					vertex.color = glm::vec3(255.0f);
				}
				
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

		boneStructures_.push_back(boneStructure);

		AnimationReturnVals* valsPtr = new AnimationReturnVals();
		valsPtr->vertices = vertices;
		valsPtr->indices = indices;
		valsPtr->boneStructIndex = boneStructures_.size()-1;
		valsPtr->animations = animationClips;
		valsPtr->succesful = true;
		resourceRegistry_[filePath] = reinterpret_cast<uintptr_t>(valsPtr);
		return *valsPtr;
	}
	else
	{
		AnimationReturnVals* valsPtr = reinterpret_cast<AnimationReturnVals*>(resourceRegistry_[filePath]);
		return *valsPtr;
	}
}

Asset* ResourceManager::createAsset(AssetCreateInfo info)
{
	TextureReturnVals textureReturnVals;
	AnimationReturnVals animationReturnVals;
	ModelReturnVals modelReturnVals;
	bool texture = false;
	bool mesh = false;
	bool animation = false;

	auto found = assetMap_.find(info);
	if (found != assetMap_.end())
	{
		return found->second;
	}

	if (info.texturePath.size() > 0)
	{
		texture = true;
		textureReturnVals = loadTextureFile(info.texturePath);
	}

	if (info.animationPath.size() > 0)
	{
		animation = true;
		animationReturnVals = loadAnimationFile(info.animationPath);
	}
	else
	{
		mesh = true;
		modelReturnVals = loadObjFile(info.modelPath);
	}

	Asset* asset = new Asset();
	if (animation)
	{
		asset->addMesh(animationReturnVals.vertices, animationReturnVals.indices);
		asset->addSkeleton(animationReturnVals.boneStructIndex);
	}
	else
	{
		asset->addMesh(modelReturnVals.vertices, modelReturnVals.indices);
	}
	if (texture)
	{
		asset->addTexture(textureReturnVals.pixels, textureReturnVals.textureWidth, textureReturnVals.textureHeight, textureReturnVals.textureChannels);
	}

	assetMap_.insert({ info, asset });
	return asset;
}

TextureReturnVals& ResourceManager::loadTextureFile(const std::string& filePath)
{
	TextureReturnVals* returnVals;
	//Look in resourceRegistry for texture.  If its not found, allocate resources and then add to registry.
	//If it is found grab pointer from registry
	auto find = resourceRegistry_.find(filePath);
	if (find == resourceRegistry_.end())
	{
		int texWidth, texHeight, texChannels;
		unsigned char* pixels;
		pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cout << "invalid texture path" << std::endl;
			assert(1 == 0);
		}
		returnVals = new TextureReturnVals(pixels, texWidth, texHeight, texChannels);

		stbi_image_free(pixels);
	
		resourceRegistry_[filePath] = reinterpret_cast<uintptr_t>(returnVals);
	}
	else
	{
		returnVals = reinterpret_cast<TextureReturnVals*>(find->second);
	}

	return *returnVals;
}

void ResourceManager::freeResource(std::string filePath)
{
	auto find = resourceRegistry_.find(filePath);
	if (find == resourceRegistry_.end())
	{
		std::cout << "Resource not in registry" << std::endl;
		assert(1 == 0);
	}
	else
	{
		delete reinterpret_cast<void*>(resourceRegistry_[filePath]);
	}
}

void ResourceManager::freeAllResources() 
{
	for (auto const& resource : resourceRegistry_)
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

bool operator==(const AssetCreateInfo& left, const AssetCreateInfo& right)
{
	return(left.animationPath == right.animationPath &&
		left.modelPath == right.modelPath &&
		left.texturePath == right.texturePath);
}
