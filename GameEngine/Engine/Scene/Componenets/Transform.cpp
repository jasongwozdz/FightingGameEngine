#include "Transform.h"
#include "../../libs/imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform(float x, float y, float z) : 
	pos_({x, y, z}),
	oldPos_({x-1, y-1, z-1})
{}

Transform::Transform(glm::vec3 pos) :
	pos_(pos),
	oldPos_(pos)
{
	oldPos_.x - 1;
}

void Transform::drawDebugGui()
{
	if (drawDebugGui_)
	{
		ImGui::Begin("Transform debug");
		ImGui::InputFloat("quat x", &rot_.x);
		ImGui::InputFloat("quat y", &rot_.y);
		ImGui::InputFloat("quat z", &rot_.z);
		ImGui::InputFloat("quat w", &rot_.w);
	}
}

void Transform::setScale(float scale)
{
	scale_.x = scale;
	scale_.y = scale;
	scale_.z = scale;
}

void Transform::applyTransformToMesh(Renderable& mesh)
{
	finalTransform_ = calculateTransform();
	mesh.ubo_.model = finalTransform_;
	return;
}

glm::mat4 Transform::calculateTransform()
{
	if (calculateTransform_)
	{
		glm::mat4 parentTrans(1.0f);
		if (parent_ && parent_->getComponent<Transform>().pos_ != parent_->getComponent<Transform>().oldPos_)
		{
			oldPos_.x -= 1;
			parentTrans = glm::translate(glm::mat4(1.0f), parent_->getComponent<Transform>().pos_);
		}

		if (pos_ != oldPos_ || scale_ != oldScale_ || rot_ != oldRot_)//only calculate new finalTransformation_ if anything has changed
		{
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), scale_);
			glm::mat4 rot = glm::toMat4(glm::normalize(rot_));
			glm::mat4 trans = glm::translate(parentTrans, pos_);

			oldPos_ = pos_;
			oldScale_ = scale_;
			oldRot_ = rot_;

			finalTransform_ = trans * rot * scale;
		}
	}
	return finalTransform_;
}
