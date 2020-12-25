#pragma once
#include "../libs/entt/entt.hpp"

class Entity
{
public:
	Entity(entt::registry& registry, std::string& name);
	Entity(const Entity& e) = default;
	virtual ~Entity();

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
	
private:
	entt::entity enttId_;
	entt::registry& registry_;
	std::string name_;
};
