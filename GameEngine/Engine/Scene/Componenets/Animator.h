#pragma once
#include <vector>
#include <ctime>
#include "glm/gtc/quaternion.hpp"
#include "../../NewRenderer/UIInterface.h"
#include "BoneStructure.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ResourceManager;

struct JointPose
{
	glm::quat quat_;
	glm::vec3 pos_;
	float scale_;
	float time_;
};

struct KeyPosition
{
	float time_;
	glm::vec3 pos_;
};

struct KeyRotation
{
	float time_;
	glm::quat quat_;
};

struct KeyScale
{
	float time_;
	float scale_;
};

struct AnimationClip
{
	std::string name_;
	float framesPerSecond_;
	float playbackRate_ = 2;
	float duration_;
	int frameCount_;
	bool isLooping_ = true;
	std::vector<std::vector<KeyPosition>> positions_;
	std::vector<std::vector<KeyRotation>> rotations_;
	std::vector<std::vector<KeyScale>> scale_;
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

	void getAnimationPoseByFrame(const AnimationClip& clip, unsigned int frameNumber, Renderable& renderable);

	void update(float currentTime, Renderable& renderable);

	int findAnimationIndexByName(const std::string& animationName);

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

	std::vector<glm::mat4> globalTransforms;

	ResourceManager& resourceManager_;

	std::vector<AnimationClip> animations_;

private:
	//TEST CONSTRUCTOR 
	Animator(std::vector<AnimationClip> animations);

	void setPose(std::vector<glm::mat4> pose, Renderable& renderable);

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
};
