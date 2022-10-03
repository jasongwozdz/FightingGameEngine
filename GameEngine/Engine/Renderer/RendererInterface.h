#pragma once

#include <vector>

#include "../Singleton.h"
#include "../Scene/Components/LightSource.h"
#include "Asset/AssetInstance.h"
#include "../Events.h"

enum class ParticleTypeVertex;
enum class ParticleTypeFragment;
class Window;
class Renderable;
class Textured;
class DebugDrawManager;
namespace UI
{
	class UIInterface;
}

class RendererInterface : Singleton<RendererInterface>
{
public:
	enum class RenderAPI
	{
		VULKAN,
		OPENGL
	};

	static RendererInterface& getSingleton();
	static RendererInterface* getSingletonPtr();

	RendererInterface(Window* window, RenderAPI api);
	virtual ~RendererInterface();

	virtual void init() = 0;
	virtual void draw(std::vector<Renderable*>& objectsToDraw, const std::vector<AssetInstance*>& assetInstancesToDraw, const DirLight& dirLight, const std::vector<PointLight>& pointLights) = 0;
	virtual void cleanup() = 0;
	virtual void uploadMesh(Renderable* mesh) = 0;
	//virtual void uploadStaticMeshData(Renderable* mesh) = 0;
	virtual void uploadTextureData(Textured* texture) = 0;
	virtual void prepareFrame() = 0;
	virtual void frameBufferResizeCallback(Events::FrameBufferResizedEvent& event) = 0;
	virtual void setDefaultShader(AssetInstance* assetInstance) const = 0;

	virtual void deleteDynamicAssetData(Entity* assetInstance) = 0;

	virtual const std::string& getParticleVertexShader(ParticleTypeVertex vertex) const = 0;
	virtual const std::string& getParticleFragmentShader(ParticleTypeFragment fragment) const = 0;

	std::vector<char> readShaderFile(const std::string& filename);

	RenderAPI api_;
	DebugDrawManager* debugDrawManager_;
	UI::UIInterface* ui_;
	Window* window_;

	GlobalUniformData globalUniformData_;
};
