#include "Transform.h"

Transform::Transform(float x, float y, float z)
	: x_(x), y_(y), z_(z){}

void Transform::applyTransformToMesh(Mesh& mesh)
{
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), (glm::vec3)*this);
	mesh.m_ubo.model = trans;
	return;
}
