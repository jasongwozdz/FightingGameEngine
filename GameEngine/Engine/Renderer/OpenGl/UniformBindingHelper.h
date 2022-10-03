#pragma once

class AssetInstance;

namespace UniformBindingHelper
{
	template<typename UniformType> void setUniformData(unsigned int shaderProgram, UniformType* data);
};
