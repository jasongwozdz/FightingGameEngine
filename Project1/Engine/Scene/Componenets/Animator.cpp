#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "Animator.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>

Animator::Animator(std::vector<AnimationClip> animations, BoneStructure* boneStructure) :
	animations_(animations)
{
	boneStructure_ = new BoneStructure(0);
	memcpy(boneStructure_, boneStructure, sizeof(*boneStructure));
	globalInverseTransform_ = boneStructure->globalInverseTransform_;
}

//CONSTRUCTOR for testing interpolation
Animator::Animator(std::vector<AnimationClip> animations ):
	animations_(animations)
{
}

Animator::Animator(const Animator& other) :
	currentAnimation_(other.currentAnimation_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_),
	boneTransforms_(other.boneTransforms_)
{
	boneStructure_ = new BoneStructure(0);
	memcpy(boneStructure_, other.boneStructure_, sizeof(*other.boneStructure_));
}

Animator::Animator(Animator&& other) :
	currentAnimation_(other.currentAnimation_),
	boneTransforms_(other.boneTransforms_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_)
{

	this->boneStructure_ = other.boneStructure_;
	other.boneStructure_ = nullptr;
}

void Animator::setAnimation(int animationIndex)
{
	int size = animations_.size();
	if (animationIndex < size && animationIndex >= -1)
	{
		currentAnimation_ = animationIndex;
		startTime_ = std::clock();
	}
	else
	{
		std::cout << "WARNING: animationIndex: " << animationIndex << " not found" << std::endl;
	}
}

Animator::~Animator()
{
	delete boneStructure_;
}

void outputMatrixToFile(std::vector<aiMatrix4x4> boneTransforms)
{
	std::string output;
	for (uint32_t i = 0; i < boneTransforms.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				output += std::to_string((boneTransforms[i])[j][k]);
			}
		}
	}
	std::cout << output << std::endl;
}

void Animator::setPose(std::vector<glm::mat4> pose, Renderable& renderable)
{
	for (int i = 0; i < pose.size(); ++i)
	{
		renderable.ubo().bones[i] =  pose[i];
	}
	for (size_t i = MAX_BONES - (MAX_BONES - pose.size()); i < MAX_BONES; i++)
	{
		renderable.ubo().bones[i] = glm::mat4(1.0f);
	}
}

void Animator::update(float currentTime, Renderable& renderable)
{
	currentTime = std::clock();
	std::vector<glm::mat4> finalTransform;

	std::vector<glm::mat4> globalTransforms;

	glm::mat4 globalTransform = boneStructure_->boneInfo_[0].invBindPose_;

	if (currentAnimation_ != -1)
	{
		AnimationClip clip = animations_[currentAnimation_];

		if (clip.isLooping_)
			localTime_ = fmod(((currentTime - startTime_)*clip.playbackRate_), clip.duration_);
		else
			localTime_ = std::clamp(((currentTime - startTime_)*clip.playbackRate_), 0.0f, clip.duration_);

		if (boneStructure_->boneInfo_[0].animated_)
			globalTransform = interpolateTransforms(1, clip);
	}

	globalTransforms.push_back(globalTransform);
	
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure_->boneInfo_[0].offset_); //model -> joint space

	glm::mat4 parent(1.0f);
	//get global positions of each joint
	for (int i = 1; i < boneStructure_->boneInfo_.size()-1; i++)
	{
		glm::mat4 currentPose(boneStructure_->boneInfo_[i].invBindPose_ );
		if (currentAnimation_ != -1 && boneStructure_->boneInfo_[i].animated_)
			currentPose = interpolateTransforms(i, animations_[currentAnimation_]);

		int parentIndex = boneStructure_->boneInfo_[i].parent_;
		if (parentIndex < finalTransform.size())
		{
			parent = globalTransforms[parentIndex];
		}
		else
		{
			parent = glm::mat4(1.0f);
			std::cout << "ERROR: parent joint not found in final transform matrix" << std::endl;
		}

		globalTransform = parent * currentPose;

		globalTransforms.push_back(globalTransform);

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure_->boneInfo_[i].offset_);
	}


	setPose(finalTransform, renderable);
}


glm::vec3 interpolateVec(glm::vec3 a, glm::vec3 b, float time)
{
	float x = a.x + time*(b.x - a.x);
	float y = a.y + time*(b.y - a.y);
	float z = a.z + time*(b.z - a.z);
	return { x, y, z };
}

glm::mat4 Animator::interpolateTransforms(int jointIndex, AnimationClip& clip)
{
	glm::mat4 pos;
	glm::mat4 rotation;
	glm::mat4 scale;

	int frameIndex = 0;
	for (int i = 0; i < clip.positions_[jointIndex].size() - 1; i++)
	{
		if (clip.positions_[jointIndex][i + 1].time_ > localTime_)
		{
			frameIndex = i;
			break;
		}
	}

KeyPosition startPos = clip.positions_[jointIndex][frameIndex];
	KeyPosition endPos = clip.positions_[jointIndex][frameIndex + 1];

	for (int i = 0; i < clip.scale_[jointIndex].size() - 1; i++)
	{
		if (clip.scale_[jointIndex][i + 1].time_ > localTime_)
		{
			frameIndex = i;
			break;
		}
	}
	KeyScale startScale = clip.scale_[jointIndex][frameIndex];
	KeyScale endScale = clip.scale_[jointIndex][frameIndex+1];

	for (int i = 0; i < clip.rotations_[jointIndex].size() - 1; i++)
	{
		if (clip.rotations_[jointIndex][i + 1].time_ > localTime_)
		{
			frameIndex = i;
			break;
		}
	}
	KeyRotation startRot = clip.rotations_[jointIndex][frameIndex];
	KeyRotation endRot = clip.rotations_[jointIndex][frameIndex+1];
	
	float normalizedTime = (localTime_ - startPos.time_) / (endPos.time_ - startPos.time_);

	float x = startPos.pos_.x + normalizedTime*(endPos.pos_.x - startPos.pos_.x);
	float y = startPos.pos_.y + normalizedTime*(endPos.pos_.y - startPos.pos_.y);
	float z = startPos.pos_.z + normalizedTime*(endPos.pos_.z - startPos.pos_.z);
	pos = glm::translate(glm::mat4(1.0f),{x, y, z});

	normalizedTime = (localTime_ - startScale.time_) / (endScale.time_ - startScale.time_);
	float s = startScale.scale_ + normalizedTime*(endScale.scale_ - startScale.scale_);
	scale = glm::scale(glm::mat4(1.0f), { s, s, s });

	normalizedTime = (localTime_ - startRot.time_) / (endRot.time_ - startRot.time_);
	glm::quat rotationQuat = glm::normalize(glm::slerp(startRot.quat_, endRot.quat_, normalizedTime));
	rotation = glm::toMat4(rotationQuat);

	glm::mat4 output = pos * rotation * scale;
	return output;
}
