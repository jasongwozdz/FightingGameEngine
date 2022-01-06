#pragma once
#include "../libs/entt/entt.hpp"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Entity
{
	friend class Scene;
public:

	Entity& operator=(const Entity& other)
	{
		enttId_ = other.enttId_;
		registry_ = std::move(other.registry_);
		name_ = other.name_;
		return *this;
	}

	template<typename T, typename ...Args>
	T& addComponent(Args&&... args)
	{
		return registry_.emplace<T>(enttId_, args...);
	}

	template<typename T>
	T& getComponent()
	{
		return registry_.get<T>(enttId_);
	}

	template<typename T>
	T* tryGetComponent()
	{
		return registry_.try_get<T>(enttId_);
	}

	template<typename T>
	void removeComponent()
	{
		registry_.remove<T>(enttId_);
	}
public:
	std::string name_;

private:
	Entity(entt::registry& registry, std::string& name);//only created through scene object
	~Entity();

	entt::entity enttId_;
	entt::registry& registry_;
};
