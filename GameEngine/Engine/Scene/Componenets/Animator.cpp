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
	int size = animations_.size();
	if (animationIndex < size && animationIndex >= -1)
	{
		currentAnimation_ = animationIndex;
		lastIndex_ = 0;
		startTime_ = std::clock();
		localTime_ = 0;
	}
	else
	{
		std::cout << "WARNING: animationIndex: " << animationIndex << " not found" << std::endl;
	}
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

void Animator::update(float currentTime, Renderable& renderable)
{
	currentTime = std::clock();
	std::vector<glm::mat4> finalTransform;

	globalTransforms.clear();

	const BoneStructure& boneStructure_ = resourceManager_.boneStructures_[boneStructureIndex_];

	glm::mat4 globalTransform = boneStructure_.boneInfo_[0].invBindPose_;

	unsigned int frameIndex = 0;
	if (currentAnimation_ != -1)
	{
		AnimationClip clip = animations_[currentAnimation_];

		for (unsigned int i = lastIndex_; i < clip.positions_[1].size() - 2; i++)
		{
			if (clip.positions_[1][i + 1].time_ > localTime_)
			{
				frameIndex = i;
				lastIndex_ = (frameIndex) % (clip.positions_[1].size()-3);
				break;
			}
		}

		if (clip.isLooping_)
			localTime_ = fmod(((currentTime - startTime_)*clip.playbackRate_), clip.duration_);
		else
			localTime_ = std::clamp(((currentTime - startTime_)*clip.playbackRate_), 0.0f, clip.duration_);

		if (boneStructure_.boneInfo_[0].animated_)
			globalTransform = interpolateTransforms(1, clip, frameIndex);
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

	globalTransforms.push_back(globalTransform);
	
	finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure_.boneInfo_[0].offset_); //model -> joint space

	glm::mat4 parent(1.0f);
	//get global positions of each joint
	for (int i = 1; i < boneStructure_.boneInfo_.size()-1; i++)
	{
		glm::mat4 currentPose(boneStructure_.boneInfo_[i].invBindPose_ );
		if (currentAnimation_ != -1 && boneStructure_.boneInfo_[i].animated_)
			currentPose = interpolateTransforms(i, animations_[currentAnimation_], frameIndex);

		int parentIndex = boneStructure_.boneInfo_[i].parent_;
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

		finalTransform.push_back(globalInverseTransform_ * globalTransform * boneStructure_.boneInfo_[i].offset_);
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
