#pragma once

#include "../RendererInterface.h"

class Asset;


typedef unsigned int ShaderId;
typedef unsigned int ShaderProgramId;
typedef unsigned int ShaderType;

class OpenGlRenderer : public RendererInterface
{
public:
	OpenGlRenderer(Window* window);
	~OpenGlRenderer();

	static OpenGlRenderer* getInstance();

	virtual void init();
	virtual void draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights);
	virtual void cleanup();
	virtual void uploadMesh(Renderable* mesh);
	virtual void uploadTextureData(Textured* texture);
	virtual void prepareFrame();
	virtual void frameBufferResizeCallback(Events::FrameBufferResizedEvent& event);
	virtual void deleteDynamicAssetData(Entity* assetInstance);

	template <typename VertexType>
	void uploadStaticMeshData(Asset* asset, const std::vector<VertexType>& verticies, const std::vector<uint32_t>& indicies);

	ShaderId compileShader(ShaderType shaderType, const std::string& shaderPath);

	ShaderProgramId linkShader(const std::vector<ShaderId>& shadersToLink);

	virtual const std::string& getParticleVertexShader(ParticleTypeVertex vertex) const;
	virtual const std::string& getParticleFragmentShader(ParticleTypeFragment fragment) const;

	template <typename UBO>
	void uploadDynamicData(AssetInstance* assetInstance);
private:

	unsigned int VAO_;
	unsigned int shaderProgram_;

	unsigned int texture1;
	unsigned int texture2;

	virtual void setDefaultShader(AssetInstance* assetInstance) const;
	void setGlobalUniformData(const DirLight& dirLight, const std::vector<PointLight>& pointLights);
	void printUniformData(int shaderProgram, const AssetInstance* assetInstance) const;
};

