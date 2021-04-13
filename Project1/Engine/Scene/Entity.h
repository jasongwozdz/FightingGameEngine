#pragma once
#include "../libs/entt/entt.hpp"

class Entity
{
public:
	Entity(entt::registry& registry, std::string& name);
	~Entity();

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
	void removeComponent()
	{
		registry_.remove<T>(enttId_);
	}
	
private:
	entt::entity enttId_;
	entt::registry& registry_;
	std::string name_;
};
