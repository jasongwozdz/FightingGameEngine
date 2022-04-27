#pragma once
#include <vector>
#include <ctime>
#include "glm/gtc/quaternion.hpp"
#include "../../Renderer/UIInterface.h"
#include "BoneStructure.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ResourceManager;
class AssetInstance;

struct AnimationClip
{
	std::string name_;
	float framesPerSecond_;
	float playbackRate_ = 2;
	float duration_;
	float durationInSeconds_;
	int frameCount_;
	bool isLooping_ = true;
	std::vector<float> times_;
	std::vector<std::vector<glm::vec3>> positions_;
	std::vector<std::vector<glm::quat>> rotations_;
	std::vector<std::vector<float>> scale_;
};

class ENGINE_API Animator
{
public:
	Animator(Animator&& animator);
	Animator(std::vector<AnimationClip> animations, int boneStructureIndex);
	Animator(const Animator& other);
	~Animator();

	void setAnimation(int animationIndex);
	void setAnimation(std::string name);
	void setAnimationTime(float timeInSeconds);
	void getAnimationPoseByFrame(const AnimationClip& clip, unsigned int frameNumber, Renderable& renderable);
	void getAnimationPoseByFrame(const AnimationClip& clip, unsigned int frameNumber,  AssetInstance* assetInstance);
	void update(float deltaTime, Renderable& renderable);
	void update(float deltaTime, AssetInstance* assetInstance);

	int findAnimationIndexByName(const std::string& animationName);
	AnimationClip* getAnimationClipByName(const std::string& animationName);

	Animator& operator=(Animator&& other)
	{
		std::cout << "Animator: Move Animator assignement" << std::endl;
		boneStructureIndex_ = other.boneStructureIndex_;
		this->currentAnimation_ = (other.currentAnimation_);
		this->animations_ = other.animations_;
		this->globalInverseTransform_ = other.globalInverseTransform_;
		this->boneTransforms_ = other.boneTransforms_;
		this->localTime_ = other.localTime_;
		return *this;
	}

	Animator& operator=(Animator const& other)
	{
		std::cout << "Animator: assignemnt" << std::endl;
		this->boneStructureIndex_ = other.boneStructureIndex_;
		this->currentAnimation_ = (other.currentAnimation_);
		this->animations_ = other.animations_;
		this->globalInverseTransform_ = other.globalInverseTransform_;
		this->boneTransforms_ = other.boneTransforms_;
		this->localTime_ = other.localTime_;
		return *this;
	}

public:
	glm::mat4 globalInverseTransform_;
	std::vector<glm::mat4> globalTransforms_;
	ResourceManager& resourceManager_;
	std::vector<AnimationClip> animations_;
	uint32_t currentFrameIndex_ = 0;

private:
	//TEST CONSTRUCTOR 
	Animator(std::vector<AnimationClip> animations);
	void setPose(const std::vector<glm::mat4>& pose, Renderable& renderable);
	glm::mat4 interpolateTransforms(int jointIndex, const AnimationClip& clip, int frameIndex);

private:
	UI::ScrollingBuffer<float, float> debugScrollingBuffer_; //DEBUG ONLY REMOVE
	int boneStructureIndex_;
	float startTime_ = 0;
	float localTime_;
	int currentAnimation_ = -1;
	std::vector<glm::mat4> boneTransforms_;
	unsigned int lastIndex_ = 0;
	bool dontUpdate_ = false; //set to true when getAnimationPoseByFrame is called
	bool timeSet_ = false;//set when setAnimationTime() is called
};
