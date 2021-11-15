#pragma once
#include <vector>
#include "../libs/entt/entt.hpp"
#include "Entity.h"
#include "../BaseCamera.h"
#include "Componenets/Transform.h"
#include "Componenets/BoneStructure.h"
#include "Componenets/Animator.h"
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
	void update(float deltaTime);
	Entity* addEntity(std::string entityName);
	entt::entity& getEntity(std::string name);
	int addCamera(BaseCamera* camera);//returns position of camera in camera vector
	void setCamera(int index);
	BaseCamera* getCurrentCamera() const;
	bool setSkybox(const std::string& path);
	void frameBufferResizedCallback(const Events::FrameBufferResizedEvent& e);
	static Scene& getSingleton();
	static Scene* getSingletonPtr();

	//void each(std::function<void(entt::entity, entt::registry)>);
	
	template<typename T>
	void each(std::function<void(entt::entity, entt::registry&, T)> fn)
	{
		auto view = registry_.view<T>();
		for (auto entity : view)
		{
			fn(entity, registry_, view.get(entity));
		}
	}

	template<typename T, typename B>
	void each(std::function<void(entt::entity, entt::registry&, T, B)> fn)
	{
		auto view = registry_.view<T,B>();
		for (auto entity : view)
		{
			fn(entity, registry_, view.get(entity));
		}
	}

	std::vector<Entity*> entitys_;

private:
	entt::registry registry_;

	std::vector<BaseCamera*> cameras_;
	int currentCamera_;

	std::vector<Renderable*> objectsToDraw_;
	int numObjectsToDraw_ = 0;

	VkRenderer* renderer_;
	class SkyBoxRenderSubsystem* skybox_;
};

