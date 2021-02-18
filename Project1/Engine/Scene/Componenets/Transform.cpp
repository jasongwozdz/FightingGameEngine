#include "Transform.h"
#include "libs/imgui/imgui.h"

Transform::Transform(float x, float y, float z)
	: x_(x), y_(y), z_(z){}

void Transform::drawDebugGui(bool draw)
{
	if (draw)
	{
		ImGui::Begin("Transform debug");
		ImGui::SliderFloat("x", &x_, 0.0f, 100.0f);
		ImGui::SliderFloat("y", &y_, 0.0f, 100.0f);
		ImGui::SliderFloat("z", &z_, 0.0f, 100.0f);
		ImGui::End();
	}
}

void Transform::applyTransformToMesh(Renderable& mesh)
{
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), (glm::vec3)*this);
	mesh.ubo_.model = trans;
	return;
}
