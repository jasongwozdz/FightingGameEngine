#pragma once
#include <vector>
#include <unordered_map>

#include "../libs/entt/entt.hpp"
#include "Entity.h"
#include "../BaseCamera.h"
#include "Components/Transform.h"
#include "Components/BoneStructure.h"
#include "Components/Animator.h"
#include "Components/Collider.h"
#include "../Renderer/Renderable.h"
#include "../Renderer/VkRenderer.h"
#include "../Renderer/Textured.h"
#include "../Renderer/SkyBoxRenderSubsystem.h"

#define MAX_DRAWN_OBJECTS 200

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Scene : Singleton<Scene>
{
public:
	Scene();
	~Scene();

	static Scene& getSingleton();
	static Scene* getSingletonPtr();

	void clearScene();

	void update(float deltaTime);
	Entity* addEntity(std::string entityName);
	void getEntities(std::string entityName, std::vector<Entity*>& outEnt);

	struct Camera& getCurrentCamera();
	void setActiveCamera(Entity* entity);

	bool setSkybox(const std::string& path);
	void calculateViewProjection(glm::mat4& viewMatrix, glm::mat4& projectionMatrix);
	Entity* createCameraEntity();

	template <typename ...components>
	std::vector<Entity*> getAllEntitiesWithComponents()
	{
		std::vector<Entity*> output;
		auto view = registry_.view<components...>();
		for (entt::entity entity : view)
		{
			output.push_back(entitys_[entity]);
		}
		return output;
	}
public:
	static float DeltaTime;
	static float DeltaTimeMs;

private:
	void initalizeDefaultCamera();
	//just for debug
	void drawCameraFrustrum(Camera& currentCamera, Transform& cameraTransform);

private:
	entt::registry registry_;
	std::unordered_map<entt::entity,Entity*> entitys_;
	std::unordered_map < std::string, std::vector<Entity*>> entityNameMap_;
	Entity* activeCamera_;
	std::vector<Renderable*> objectsToDraw_;
	VkRenderer* renderer_;
	class SkyBoxRenderSubsystem* skybox_;
	class BoxCollisionManager* boxCollisionManager_;
};

