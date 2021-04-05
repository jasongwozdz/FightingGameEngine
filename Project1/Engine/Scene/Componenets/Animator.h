#pragma once
#include "assimp/scene.h"
#include "Scene/Componenets/BoneStructure.h"
#include <vector>
#include "glm/gtc/quaternion.hpp"
#include <ctime>

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

struct AnimationSample
{
	std::vector<JointPose> jointPose_;
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

class Animator
{
public:

	//UNIT TESTS FIXTURES
	friend class AnimatorTestFixture;
	//END UNIT TESTS

	Animator(Animator&& animator);

	Animator(std::vector<AnimationClip> animations, BoneStructure* boneStructure);

	Animator(const Animator& other);

	void setAnimation(int animationIndex);

	~Animator();

	Animator& operator=(Animator const& other)
	{
		this->boneStructure_ = new BoneStructure(0);
		memcpy(this->boneStructure_, other.boneStructure_, sizeof(*other.boneStructure_));
		this->currentAnimation_ = (other.currentAnimation_);
		this->animations_ = other.animations_;
		this->globalInverseTransform_ = other.globalInverseTransform_;
		this->boneTransforms_ = other.boneTransforms_;
		this->localTime_ = other.localTime_;
		return *this;
	}

	bool playAnimation(float currentTime, std::string animationName);

	void update(float currentTime, Renderable& renderable);

private:
	//TEST CONSTRUCTOR 
	Animator(std::vector<AnimationClip> animations);

	BoneStructure* boneStructure_;

	std::vector<AnimationClip> animations_;

	float startTime_ = 0;
	
	float localTime_;

	int currentAnimation_ = -1;

	std::vector<glm::mat4> boneTransforms_;

	glm::mat4 globalInverseTransform_;

	void setPose(std::vector<glm::mat4> pose, Renderable& renderable);

	glm::mat4 interpolateTransforms(int jointIndex, AnimationClip& clip);
};
