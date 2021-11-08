#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Animator.h"
#include "../../ResourceManager.h"
#include "../../libs/imgui/imgui.h"
#include "assimp/scene.h"
#include <stb_image.h>

Animator::Animator(std::vector<AnimationClip> animations, int boneStructureIndex) :
	animations_(animations), boneStructureIndex_(boneStructureIndex), resourceManager_(ResourceManager::getSingleton())
{
	globalInverseTransform_ = resourceManager_.boneStructures_[boneStructureIndex_].globalInverseTransform_;
;
}

//CONSTRUCTOR for testing interpolation
Animator::Animator(std::vector<AnimationClip> animations ):
	animations_(animations), resourceManager_(ResourceManager::getSingleton())
{
}

Animator::Animator(const Animator& other) :
	currentAnimation_(other.currentAnimation_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_),
	boneTransforms_(other.boneTransforms_),
	resourceManager_(other.resourceManager_)
{
	std::cout << "Animator: Copy constructor" << std::endl;
	boneStructureIndex_ = other.boneStructureIndex_;
}

Animator::Animator(Animator&& other) :
	currentAnimation_(other.currentAnimation_),
	boneTransforms_(other.boneTransforms_),
	animations_(other.animations_),
	globalInverseTransform_(other.globalInverseTransform_),
	localTime_(other.localTime_),
	resourceManager_(other.resourceManager_)
{
	std::cout << "Animator: Move constructor" << std::endl;
	boneStructureIndex_ = other.boneStructureIndex_;
}

void Animator::setAnimation(int animationIndex)
{
	dontUpdate_ = false;
	int size = animations_.size();
	if (animationIndex < size && animationIndex >= -1)
	{
		currentAnimation_ = animationIndex;
		lastIndex_ = 0;
		//startTime_ = std::clock();
		startTime_ = 0;
		localTime_ = 0;
	}
	else
	{
		std::cout << "WARNING: animationIndex: " << animationIndex << " not found" << std::endl;
	}
}

void Animator::setAnimation(std::string name)
{
	dontUpdate_ = false;
	for (int i = 0; i < animations_.size(); i++)
	{
		if (animations_[i].name_ == name)
		{
			currentAnimation_ = i;
			lastIndex_ = 0;
			startTime_ = 0;
			localTime_ = 0;
			currentFrameIndex_ = 0;
			return;
		}
	}
	std::cout << "Error: animation not found" << std::endl;
}

void Animator::getAnimationPoseByFrame(const AnimationClip& clip, unsigned int frameNumber, Renderable& renderable)
{
	if (frameNumber > clip.frameCount_)
	{
		std::cout << "Error: frame number out of bounds" << std::endl;
		return;
	}

	const BoneStructure& boneStructure = resourceManager_.boneStructures_[boneStructureIndex_];

	globalTransforms_.clear();

	glm::mat4 globalTransform = boneStructure.boneInfo_[0].invBindPose_;
	glm::mat4 transform;
	std::vector<glm::mat4> finalTransform;

	if (boneStructure.boneInfo_[0].animated_)
	{
		globalTransform = 
			glm::translate(glm::mat4(1.0f), clip.positions_[1][frameNumber].pos_) *
			glm::toMat4(clip.rotations_[1][frameNumber].quat_) * 
			glm::scale(glm::mat4(1.0f), glm::vec3(clip.scale_[1][frameNumber].scale_));
	}

	globalTransforms_.push_back(globalTransform);
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[0].offset_);

	for (int i = 1; i < boneStructure.boneInfo_.size() - 1; i++)
	{
		glm::mat4 transform = boneStructure.boneInfo_[i].invBindPose_;
		if (boneStructure.boneInfo_[i].animated_)
		{
			transform =
				glm::translate(glm::mat4(1.0f), clip.positions_[i][frameNumber].pos_) *
				glm::toMat4(clip.rotations_[i][frameNumber].quat_) *
				glm::scale(glm::mat4(1.0f), glm::vec3(clip.scale_[i][frameNumber].scale_));
		}

		glm::mat4 parent(1.0f);
		int parentIndex = boneStructure.boneInfo_[i].parent_;
		if (parentIndex < finalTransform.size())
		{
			parent = globalTransforms_[parentIndex];
		}
		else
		{
			parent = glm::mat4(1.0f);
			std::cout << "ERROR: parent joint not found in final transform matrix" << std::endl;
		}

		globalTransform = parent * transform;
		globalTransforms_.push_back(globalTransform);

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[i].offset_);
	}
	setPose(finalTransform, renderable);
	dontUpdate_ = true;
}

Animator::~Animator()
{
	//delete boneStructure_;
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
		renderable.ubo_.bones[i] =  pose[i];
	}
	for (size_t i = MAX_BONES - (MAX_BONES - pose.size()); i < MAX_BONES; i++)
	{
		renderable.ubo_.bones[i] = glm::mat4(1.0f);
	}
}

void Animator::update(float deltaTime, Renderable& renderable)
{
	if (dontUpdate_)
	{
		return;
	}

	std::vector<glm::mat4> finalTransform;

	globalTransforms_.clear();

	const BoneStructure& boneStructure = resourceManager_.boneStructures_[boneStructureIndex_];

	glm::mat4 globalTransform = boneStructure.boneInfo_[0].invBindPose_;

	if (currentAnimation_ != -1)
	{
		const AnimationClip& clip = animations_[currentAnimation_];

		localTime_ += deltaTime * 0.001 * clip.playbackRate_;//in seconds
		//std::cout << "DeltaTime " << deltaTime << std::endl;

		if (clip.isLooping_)
			localTime_ = fmod(((localTime_)), clip.durationInSeconds_);
		else
			localTime_ = std::clamp(((localTime_)), 0.0f, clip.durationInSeconds_);

		for (unsigned int i = 0; i < clip.frameCount_-1; i++)
		{
			if (clip.positions_[2][i + 1].time_ > localTime_)
			{
				currentFrameIndex_ = i;
				//lastIndex_ = (frameIndex) % (clip.positions_[2].size()-3);
				break;
			}
		}

		if (boneStructure.boneInfo_[0].animated_)
			globalTransform = interpolateTransforms(1, clip, currentFrameIndex_);
	}

#define DEBUG 0
#if DEBUG == 1
	if (ImGui::Begin("Animator Debug"))
	{
		debugScrollingBuffer_.add( currentTime, localTime_ );

		ImPlot::SetNextPlotLimitsX(currentTime - 3000.0f, currentTime, ImGuiCond_Always);
		if (ImPlot::BeginPlot("AnimatorTiminig", "time", "LocalTime"))
		{
			ImPlot::PlotLine("localTime", debugScrollingBuffer_.bufferX.data(), debugScrollingBuffer_.bufferY.data(), debugScrollingBuffer_.bufferX.size());
			ImPlot::EndPlot();
		}
		ImGui::End();
	}
#endif

	globalTransforms_.push_back(globalTransform);
	
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[0].offset_); //model -> joint space

	glm::mat4 parent(1.0f);
	//get global positions of each joint
	for (int i = 1; i < boneStructure.boneInfo_.size()-1; i++)
	{
		glm::mat4 currentPose(boneStructure.boneInfo_[i].invBindPose_ );
		if (currentAnimation_ != -1 && boneStructure.boneInfo_[i].animated_)
			currentPose = interpolateTransforms(i, animations_[currentAnimation_], currentFrameIndex_);

		int parentIndex = boneStructure.boneInfo_[i].parent_;
		if (parentIndex < finalTransform.size())
		{
			parent = globalTransforms_[parentIndex];
		}
		else
		{
			parent = glm::mat4(1.0f);
			std::cout << "ERROR: parent joint not found in final transform matrix" << std::endl;
		}

		globalTransform = parent * currentPose;

		globalTransforms_.push_back(globalTransform);

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure.boneInfo_[i].offset_);
	}

	setPose(finalTransform, renderable);
}

glm::mat4 Animator::interpolateTransforms(int jointIndex, const AnimationClip& clip, int frameIndex)
{
	glm::mat4 pos;
	glm::mat4 rotation;
	glm::mat4 scale;

	KeyPosition startPos = clip.positions_[jointIndex][frameIndex];
	KeyPosition endPos = clip.positions_[jointIndex][frameIndex + 1];

	KeyScale startScale = clip.scale_[jointIndex][frameIndex];
	KeyScale endScale = clip.scale_[jointIndex][frameIndex+1];

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

int Animator::findAnimationIndexByName(const std::string& animationName)
{
	for (int i = 0; i < animations_.size(); i++)
	{
		if (animations_[i].name_ == animationName)
		{
			return i;
		}
	}
	return -1;
}

AnimationClip* Animator::getAnimationClipByName(const std::string& animationName)
{
	AnimationClip* clip = nullptr;
	int index = findAnimationIndexByName(animationName);
	if (index != -1)
	{
		clip = &animations_[index];
	}
	return clip;
}
