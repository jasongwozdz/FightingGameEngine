#include "Transform.h"
#include "libs/imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform(float x, float y, float z)
	: x_(x), y_(y), z_(z){}

void Transform::drawDebugGui()
{
	if (drawDebugGui_)
	{
		ImGui::Begin("Transform debug");
		ImGui::InputFloat("x", &x_);
		ImGui::InputFloat("y", &y_);
		ImGui::InputFloat("z", &z_);
		//ImGui::SliderFloat("x", &x_, 0.0f, 100.0f);
		//ImGui::SliderFloat("y", &y_, 0.0f, 100.0f);
		//ImGui::SliderFloat("z", &z_, 0.0f, 100.0f);
		ImGui::InputFloat("scale", &scaleX_);
		scaleY_ = scaleX_;
		scaleZ_ = scaleX_;
		ImGui::End();
	}
}

void Transform::setScale(float scale)
{
	scaleX_ = scale;
	scaleY_ = scale;
	scaleZ_ = scale;
}

void Transform::applyTransformToMesh(Renderable& mesh)
{
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), { scaleX_, scaleY_, scaleZ_ });
	glm::mat4 trans = glm::translate(scale, (glm::vec3)*this);
	mesh.ubo_.model = trans;
	return;
}
