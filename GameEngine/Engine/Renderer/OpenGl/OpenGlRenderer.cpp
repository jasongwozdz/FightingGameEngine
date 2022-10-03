#include "OpenGlRenderer.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "../../ResourceManager.h"

#include "../../DebugDrawManager.h"
#include "../UIInterface.h"

#include "../../Vertex.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Components/Transform.h"
#include "../../Scene/Components/Camera.h"

#include "../../Particles/ParticleTypes.h"

#include "UniformBindingHelper.h"

#define CHECK_FOR_ERROR() \
{\
	GLenum error = glGetError(); \
	_ASSERT(error == GL_NO_ERROR);\
}

OpenGlRenderer::OpenGlRenderer(Window * window) : 
	RendererInterface(window, RenderAPI::OPENGL)
{}

OpenGlRenderer::~OpenGlRenderer()
{
	cleanup();
}

OpenGlRenderer* OpenGlRenderer::getInstance()
{
	RendererInterface* renderer = RendererInterface::getSingletonPtr();
	if (renderer->api_ == RendererInterface::RenderAPI::OPENGL)
	{
		return static_cast<OpenGlRenderer*>(renderer);
	}
	else
	{
		_ASSERTE(false, ("Trying to get Opengl renderer but vulkan api was selected"));
	}
	return nullptr;
}

void OpenGlRenderer::init()
{
	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		//failed to load glad
		_ASSERTE(false);
		return;
	}

	glViewport(0, 0, window_->windowInfo_.width, window_->windowInfo_.height);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);


	debugDrawManager_ = new DebugDrawManager();
	ui_ = new UI::UIInterface();
}

void OpenGlRenderer::setGlobalUniformData(const DirLight& dirLight, const std::vector<PointLight>& pointLights)
{
	globalUniformData_.dirLightData = dirLight.uniformData_;

	globalUniformData_.numLights = pointLights.size();
	for (int i = 0; i < globalUniformData_.numLights; i++)
	{
		globalUniformData_.pointLightData[i] = pointLights[i].uniformData_;
	}

	globalUniformData_.viewPos = Scene::getSingleton().getCurrentCamera().entity_->getComponent<Transform>().position_;
}

void OpenGlRenderer::printUniformData(int shaderProgram, const AssetInstance* assetInstance) const
{
	std::cout << "------------------------SHADER PROGRAM UNIFORM DATA----------------------------" << std::endl;
	std::cout << "Vertex Shader " << assetInstance->createInfo_.vertexShader << std::endl;
	std::cout << "Fragment Shader " << assetInstance->createInfo_.fragmentShader << std::endl;

	static const int buffSize = 512;
	char* uniformName = new char[buffSize];
	int params = 0;
	glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &params);

	for (int paramIdx = 0; paramIdx < params; paramIdx++)
	{
		int buffLength = 0;
		glGetActiveUniformName(shaderProgram, paramIdx, buffSize, &buffLength, uniformName);
		for (int i = 0; i < buffLength; i++)
		{
			std::cout << uniformName[i];
		}
		std::endl(std::cout);
	}
	delete[] uniformName;

	std::cout << "------------------------END SHADER PROGRAM UNIFORM DATA----------------------------" << std::endl;
}

void OpenGlRenderer::draw(std::vector<Renderable*>&, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight & dirLight, const std::vector<PointLight>& pointLights)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	setGlobalUniformData(dirLight, pointLights);

	for (const AssetInstance* assetInstance : assetInstancesToDraw)
	{
		glUseProgram(assetInstance->shaderProgram_);
		CHECK_FOR_ERROR();

		glBindVertexArray(assetInstance->asset_->oMesh_.VAO);
		CHECK_FOR_ERROR();

		if (assetInstance->createInfo_.lightingEnabled)
		{
			UniformBindingHelper::setUniformData(assetInstance->shaderProgram_, &globalUniformData_);
			CHECK_FOR_ERROR();
		}

		if (assetInstance->getSkeleton())
		{
			UniformBindingHelper::setUniformData(assetInstance->shaderProgram_, static_cast<MVPBoneData*>(assetInstance->data_.ubo_));
			CHECK_FOR_ERROR();
		}
		else
		{
			UniformBindingHelper::setUniformData(assetInstance->shaderProgram_, (assetInstance->data_.ubo_));
			CHECK_FOR_ERROR();
		}

		if (assetInstance->asset_->texture_)
		{
			glActiveTexture(GL_TEXTURE0);
			UniformBindingHelper::setUniformData(assetInstance->shaderProgram_, (assetInstance->asset_->texture_));
			glBindTexture(GL_TEXTURE_2D, assetInstance->asset_->texture_->resources_.textureId_);
			CHECK_FOR_ERROR();
		}

		CHECK_FOR_ERROR();
		glDrawElements(GL_TRIANGLES, assetInstance->asset_->mesh_.numInidicies_, GL_UNSIGNED_INT, 0);
	}

	ui_->renderFrame(nullptr);
}

void OpenGlRenderer::cleanup()
{
	delete debugDrawManager_;
}

void OpenGlRenderer::uploadMesh(Renderable * mesh)
{
}

template
void OpenGlRenderer::uploadStaticMeshData<NonAnimVertex>(Asset* asset, const std::vector<NonAnimVertex>& verticies, const std::vector<uint32_t>& indicies);

template
void OpenGlRenderer::uploadStaticMeshData<Vertex>(Asset* asset, const std::vector<Vertex>& verticies, const std::vector<uint32_t>& indicies);

void OpenGlRenderer::uploadTextureData(Textured * texture)
{
	glGenTextures(1, &texture->resources_.textureId_);
	glBindTexture(GL_TEXTURE_2D, texture->resources_.textureId_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->textureWidth_, texture->textureHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels_.data());
	glGenerateMipmap(GL_TEXTURE_2D);

}

void OpenGlRenderer::prepareFrame()
{
	ui_->prepareFrame();
}

void OpenGlRenderer::deleteDynamicAssetData(Entity * assetInstance)
{

}

ShaderId OpenGlRenderer::compileShader(ShaderType shaderType, const std::string & shaderPath)
{
	std::vector<char> shaderVec = readShaderFile(shaderPath);
	const char* shaderSource = shaderVec.data();

	ShaderId shader = glCreateShader(shaderType);
	CHECK_FOR_ERROR();

	glShaderSource(shader, 1, &shaderSource, NULL);
	CHECK_FOR_ERROR();
	glCompileShader(shader);
	CHECK_FOR_ERROR();

	//check if vertex shader compiled succesfully
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "Shader Compile failed\n" << infoLog << std::endl;
		_ASSERT(false); 
	}

	return shader;
}

ShaderProgramId OpenGlRenderer::linkShader(const std::vector<ShaderId>& shadersToLink)
{
	ShaderProgramId shaderProgram = glCreateProgram();
	CHECK_FOR_ERROR();
	for (ShaderId shader : shadersToLink)
	{
		glAttachShader(shaderProgram, shader);
		CHECK_FOR_ERROR();
	}

	glLinkProgram(shaderProgram);
	//check to see if there were any issues linking the shaders
	int  success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "SHADERLINKING FAILED\n" << infoLog << std::endl;
		_ASSERT(false);//shader not succesfully linked
	}
	return shaderProgram;
}

const std::string& OpenGlRenderer::getParticleVertexShader(ParticleTypeVertex vertex) const
{
	static const std::string s_particleShaderPaths[static_cast<int>(ParticleTypeVertex::MAX)] = 
	{
		"./shaders/OpenGLShaders/shaderLighting.vert.glsl",
	};
	return s_particleShaderPaths[static_cast<int>(vertex)];
}

const std::string& OpenGlRenderer::getParticleFragmentShader(ParticleTypeFragment fragment) const
{

	static const std::string s_particleShaderPaths[static_cast<int>(ParticleTypeFragment::MAX)] = 
	{
		"./shaders/OpenGLShaders/shaderLighting.frag.glsl",
	};
	return s_particleShaderPaths[static_cast<int>(fragment)];
}

void OpenGlRenderer::frameBufferResizeCallback(Events::FrameBufferResizedEvent& event)
{
	glViewport(0, 0, event.width_, event.height_);
}

template<typename VertexType>
inline void OpenGlRenderer::uploadStaticMeshData(Asset* asset, const std::vector<VertexType>& verticies, const std::vector<uint32_t>& indicies)
{
	glGenVertexArrays(1, &asset->oMesh_.VAO);
	CHECK_FOR_ERROR();
	glBindVertexArray(asset->oMesh_.VAO);
	//setup EBO
	//allows for verticies+indicies
	glGenBuffers(1, &asset->oMesh_.EBO);
	CHECK_FOR_ERROR();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asset->oMesh_.EBO);
	CHECK_FOR_ERROR();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);
	CHECK_FOR_ERROR();

	//setup VBO
	glGenBuffers(1, &asset->oMesh_.VBO);
	CHECK_FOR_ERROR();
	glBindBuffer(GL_ARRAY_BUFFER, asset->oMesh_.VBO);
	CHECK_FOR_ERROR();
	glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(VertexType), verticies.data(), GL_STATIC_DRAW);
	CHECK_FOR_ERROR();

	VertexUtil::setupVertexAttribPointers<VertexType>();
	CHECK_FOR_ERROR();

	glBindVertexArray(0);
	CHECK_FOR_ERROR();
}

void OpenGlRenderer::setDefaultShader(AssetInstance* assetInstance) const
{
	bool hasSkeleton = assetInstance->getSkeleton();
	bool lightingEnabled = assetInstance->createInfo_.lightingEnabled;
	bool hasTexture = assetInstance->asset_->texture_;
	
	if (hasSkeleton)
	{
		if (lightingEnabled && hasTexture)
		{
			assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/animatedMeshLighting.vert.glsl";
			assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/animatedMeshLighting.frag.glsl";
		}
		else if (lightingEnabled)
		{
			//Need to handle an animated mesh with no texture but lighting enabled
			_ASSERT(false);
		}
		else if (hasTexture)
		{
			assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/animatedMesh.vert.glsl";
			assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/animatedMesh.frag.glsl";
		}
		else
		{
			//Need to handle an animated mesh with no texture
			_ASSERT(false);
		}
	}
	else if (hasTexture)
	{
		if (lightingEnabled)
		{
			assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/texturedMeshLighting.vert.glsl";
			assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/texturedMeshLighting.frag.glsl";
		}
		else
		{
			assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/texturedMesh.vert.glsl";
			assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/texturedMesh.frag.glsl";
		}
	}
	else if (lightingEnabled)
	{
		assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/shaderLighting.vert.glsl";
		assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/shaderLighting.frag.glsl";
	}
	else
	{
		assetInstance->createInfo_.vertexShader = "./shaders/OpenGLShaders/vert.spv";
		assetInstance->createInfo_.fragmentShader = "./shaders/OpenGLShaders/frag.spv";
	}
}

template void OpenGlRenderer::uploadDynamicData<MVPBoneData>(AssetInstance* assetInstance);

template void OpenGlRenderer::uploadDynamicData<MVP>(AssetInstance* assetInstance);

template <typename UBO>
void OpenGlRenderer::uploadDynamicData(AssetInstance* assetInstance)
{
	std::vector<ShaderId> shadersToLink;
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, assetInstance->createInfo_.vertexShader);
	shadersToLink.push_back(vertexShader);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, assetInstance->createInfo_.fragmentShader);
	shadersToLink.push_back(fragmentShader);
	assetInstance->shaderProgram_ = linkShader(shadersToLink);
	glUseProgram(assetInstance->shaderProgram_);
	CHECK_FOR_ERROR();

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	printUniformData(assetInstance->shaderProgram_, assetInstance);
}