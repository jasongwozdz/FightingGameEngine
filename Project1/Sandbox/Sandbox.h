#pragma once
#include "../Application.h"
#include "../Engine/Scene/Scene.h"

class Sandbox : public Application
{
public:
	virtual ~Sandbox();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
private:
	BaseCamera* baseCamera;
	bool drawDebug;

	void initScene();
	void onEvent(Events::Event& e);
	void handleMouseClick(Events::MousePressedEvent& e);
	void handleMouseMove(Events::MouseMoveEvent& e);
	void handleKeyButtonDown(Events::KeyPressedEvent& e);
	void transformVel(entt::entity e, entt::registry& r, ...);
};

