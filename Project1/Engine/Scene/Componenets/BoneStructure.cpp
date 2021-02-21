#include "BoneStructure.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

BoneStructure::BoneStructure(std::map<std::string, uint32_t> boneMapping, std::vector<BoneInfo> boneInfo, Renderable& renderable) :
	boneMapping_(boneMapping),
	boneInfo_(boneInfo),
	renderable_(renderable)
{}

void BoneStructure::setPose(std::vector<aiMatrix4x4> pose)
{
	for (int i = 0; i < pose.size(); ++i)
	{
		renderable_.ubo_.bones[i] =  glm::transpose(glm::make_mat4(&pose[i].a1));
	}
	for (size_t i = MAX_BONES - (MAX_BONES - pose.size()); i < MAX_BONES; i++)
	{
		renderable_.ubo_.bones[i] = glm::mat4(1.0f);
	}
}
