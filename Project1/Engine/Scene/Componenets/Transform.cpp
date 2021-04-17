#include "Transform.h"
#include "libs/imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform(float x, float y, float z) : 
	x_(x), y_(y), z_(z){}

Transform::Transform(glm::vec3 pos) :
	x_(pos.x), y_(pos.y), z_(pos.z){}

void Transform::drawDebugGui()
{
	if (drawDebugGui_)
	{
		ImGui::Begin("Transform debug");
		ImGui::InputFloat("x", &x_);
		ImGui::InputFloat("y", &y_);
		ImGui::InputFloat("z", &z_);
		ImGui::InputFloat("quat x", &quaternion_.x);
		ImGui::InputFloat("quat y", &quaternion_.y);
		ImGui::InputFloat("quat z", &quaternion_.z);
		ImGui::InputFloat("quat w", &quaternion_.w);

		ImGui::InputFloat("scale", &scaleX_);
		scaleY_ = scaleX_;
		scaleZ_ = scaleX_;
		ImGui::End();
	}
}

void Transform::flipScale()
{
	scaleX_ *= -1;
	scaleY_ *= -1;
	scaleZ_ *= -1;
}

void Transform::setScale(float scale)
{
	scaleX_ = scale;
	scaleY_ = scale;
	scaleZ_ = scale;
}

void Transform::applyTransformToMesh(Renderable& mesh)
{
	glm::mat4 finalTranform = calculateTransform();
	
	mesh.ubo_.model = finalTranform;
	return;
}

glm::vec3 Transform::getPosition()
{
	return *this;
}

void Transform::setPosition(glm::vec3 pos)
{
	x_ = pos.x;
	y_ = pos.y;
	z_ = pos.z;
}

glm::mat4 Transform::calculateTransform()
{
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), (glm::vec3)*this);
	glm::mat4 rotationTrans = glm::toMat4(glm::normalize(quaternion_)) * trans;
	glm::mat4 rotationTransScale = glm::scale(rotationTrans, { scaleX_, scaleY_, scaleZ_ });

	return rotationTransScale;
}

void applyAxisAngleRotation(float rotation, glm::vec3 axis)
{

}
