#pragma once
#include <vector>
#include <ctime>
#include "glm/gtc/quaternion.hpp"
#include "../../NewRenderer/UIInterface.h"
#include "assimp/scene.h"
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
	float playbackRate_ = .0005;
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

	glm::mat4 globalInverseTransform_;

	std::vector<glm::mat4> globalTransforms;

	//UNIT TESTS FIXTURES
	friend class AnimatorTestFixture;
	//END UNIT TESTS

	Animator(Animator&& animator);

	Animator(std::vector<AnimationClip> animations, int boneStructureIndex);

	Animator(const Animator& other);

	void setAnimation(int animationIndex);

	ResourceManager& resourceManager_;

	~Animator();

	Animator& operator=(Animator&& other)
	{

		std::cout << "Animator: Move Animator assignement" << std::endl;
		boneStructureIndex_ = other.boneStructureIndex_;
		//this->boneStructure_ = other.boneStructure_;
		//other.boneStructure_ = nullptr;
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
		//this->boneStructure_ = new BoneStructure(other.boneStructure_->boneInfo_.size());
		//memcpy(this->boneStructure_, other.boneStructure_, sizeof(BoneStructure));
		this->currentAnimation_ = (other.currentAnimation_);
		this->animations_ = other.animations_;
		this->globalInverseTransform_ = other.globalInverseTransform_;
		this->boneTransforms_ = other.boneTransforms_;
		this->localTime_ = other.localTime_;
		return *this;
	}

	void update(float currentTime, Renderable& renderable);

private:
	UI::ScrollingBuffer<float, float> debugScrollingBuffer_; //DEBUG ONLY REMOVE

	//TEST CONSTRUCTOR 
	Animator(std::vector<AnimationClip> animations);

	int boneStructureIndex_;

	std::vector<AnimationClip> animations_;

	float startTime_ = 0;
	
	float localTime_;

	int currentAnimation_ = -1;

	std::vector<glm::mat4> boneTransforms_;

	void setPose(std::vector<glm::mat4> pose, Renderable& renderable);

	glm::mat4 interpolateTransforms(int jointIndex, const AnimationClip& clip, int frameIndex);

	unsigned int lastIndex_ = 0;
};
