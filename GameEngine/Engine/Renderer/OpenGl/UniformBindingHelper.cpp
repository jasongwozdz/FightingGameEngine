#include "UniformBindingHelper.h"

#include <string>

#include "../Asset/AssetTypes.h"
#include "../../Renderer/Textured.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CHECK_FOR_ERROR() \
{\
	GLenum error = glGetError(); \
	_ASSERT(error == GL_NO_ERROR);\
}

int getLocationOfUniform(unsigned int shaderProgram, const std::string& uniformName, bool unused = false)
{
	int index = glGetUniformLocation(shaderProgram, uniformName.c_str());
	//We only want to asset if we know a uniform is not unused
	if (!unused)
	{
		_ASSERT(index != -1);
	}
	return index;
}

template<> void UniformBindingHelper::setUniformData<MVP>(unsigned int shaderProgram, MVP* data)
{
	int location = getLocationOfUniform(shaderProgram, "ubo.model");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->model));
	CHECK_FOR_ERROR();
	
	location = getLocationOfUniform(shaderProgram, "ubo.view");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->view));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram, "ubo.proj");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->projection));
	CHECK_FOR_ERROR();
}

template<> void UniformBindingHelper::setUniformData<MVPBoneData>(unsigned int shaderProgram, MVPBoneData* data)
{
	int location = getLocationOfUniform(shaderProgram, "ubo.model");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->model));
	CHECK_FOR_ERROR();
	
	location = getLocationOfUniform(shaderProgram, "ubo.view");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->view));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram, "ubo.proj");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data->projection));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram, "ubo.bones[0]");
	glUniformMatrix4fv(location, MAX_BONES, GL_FALSE, glm::value_ptr(data->bones_[0]));
	CHECK_FOR_ERROR();
}

template<> void UniformBindingHelper::setUniformData<Textured>(unsigned int shaderProgram, Textured* data)
{
	glUniform1i(getLocationOfUniform(shaderProgram, "texSampler"), 0);
}

template<> void UniformBindingHelper::setUniformData<GlobalUniformData>(unsigned int shaderProgram, GlobalUniformData* data)
{
	int location = getLocationOfUniform(shaderProgram, "uniformData.viewPos", true);
	glUniform3fv(location, 1, glm::value_ptr(data->viewPos));
	CHECK_FOR_ERROR();

	std::string locationStrPrefix = "uniformData.pointLight[";
	for (int lightIdx = 0; lightIdx < data->numLights; lightIdx++)
	{
		std::string currentIndexStr = locationStrPrefix + std::to_string(lightIdx) + "].";
		const PointLightUniformData& currentLight = data->pointLightData[lightIdx];
		int location = getLocationOfUniform(shaderProgram,  currentIndexStr + "constant");
		glUniform1f(location, currentLight.constant);
		CHECK_FOR_ERROR();

		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "linear");
		glUniform1f(location, currentLight.linear);
		CHECK_FOR_ERROR();

		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "quadratic");
		glUniform1f(location, currentLight.quadratic);
		CHECK_FOR_ERROR();

		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "position");
		glUniform3fv(location, 1, glm::value_ptr(currentLight.position));
		CHECK_FOR_ERROR();

		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "ambient");
		glUniform3fv(location, 1, glm::value_ptr(currentLight.ambient));
		CHECK_FOR_ERROR();

		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "diffuse");
		glUniform3fv(location, 1, glm::value_ptr(currentLight.diffuse));
		CHECK_FOR_ERROR();

		//TODO: make this used
		location = getLocationOfUniform(shaderProgram,  currentIndexStr + "specular", true);
		glUniform3fv(location, 1, glm::value_ptr(currentLight.specular));
		CHECK_FOR_ERROR();
	}

	const DirLightUniformData& dirLightData = data->dirLightData;
	location = getLocationOfUniform(shaderProgram,  "uniformData.dirLight.direction");
	glUniform3fv(location, 1, glm::value_ptr(dirLightData.direction));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram,  "uniformData.dirLight.ambient");
	glUniform3fv(location, 1, glm::value_ptr(dirLightData.ambient));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram,  "uniformData.dirLight.diffuse");
	glUniform3fv(location, 1, glm::value_ptr(dirLightData.diffuse));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram,  "uniformData.dirLight.specular", true);
	glUniform3fv(location, 1, glm::value_ptr(dirLightData.specular));
	CHECK_FOR_ERROR();

	location = getLocationOfUniform(shaderProgram, "uniformData.numLights");
	glUniform1i(location, data->numLights);
	CHECK_FOR_ERROR();
}
