#include "RendererInterface.h"
#include <fstream>

template<> RendererInterface* Singleton<RendererInterface>::msSingleton = 0;

RendererInterface& RendererInterface::getSingleton()
{
	_ASSERT_EXPR(msSingleton, ("RendererInterface not initalized can't get singleton"));
	return *msSingleton;
}

RendererInterface* RendererInterface::getSingletonPtr()
{
	_ASSERT_EXPR(msSingleton, ("RendererInterface not initalized can't get singleton"));
	return msSingleton;
}

RendererInterface::RendererInterface(Window * window, RenderAPI api) : 
	window_(window),
	api_(api)
{
	msSingleton = this;//set singleton
}

RendererInterface::~RendererInterface()
{
}

std::vector<char> RendererInterface::readShaderFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open() || !file.good()) {
		std::cout << "ERROR: could not find shader file" << std::endl;
		_ASSERT(false);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	buffer.push_back('\0');
	return buffer;
};
