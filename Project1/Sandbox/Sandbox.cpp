#include <iostream>
#include "Sandbox.h"
#include "../EntryPoint.h"

Sandbox::~Sandbox()
{
	delete baseCamera;
}

void Sandbox::transformVel(entt::entity e, entt::registry& r, ...)
{
	
}

void Sandbox::initScene()
{
	scene_ = new Scene();
	Entity& e = scene_->addEntity("Temp");

	Transform& t = e.addComponent<Transform>( 1.0f,1.0f,1.0f );
	t.x_ = 2.0f;


	std::string modelPath = "./Models/viking_room.obj";
	std::string texturePath = "./Textures/viking_room.png";
	glm::vec3 pos = { 0, 0, 0 };
	int houseId = (gObjectManager->addGameObject(modelPath, texturePath, pos,BASIC_PIPELINE));
	baseCamera = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, {0.0f, 0.0f, 1.0f});
	gObjectManager->addCamera(baseCamera);
	debugDrawManager->drawGrid({ 255, 255, 255 }, true);
}

void Sandbox::handleMouseClick(Events::MousePressedEvent& e)
{
	std::cout << "Mouse clicked" << std::endl;
}

void Sandbox::handleMouseMove(Events::MouseMoveEvent& e)
{
	std::cout << "mouseMoved" << std::endl;
	baseCamera->updateMouse({ e.mouseX, e.mouseY });
}

void Sandbox::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::MousePressedEvent>(std::bind(&Sandbox::handleMouseClick, this, std::placeholders::_1));
	d.dispatch<Events::MouseMoveEvent>(std::bind(&Sandbox::handleMouseMove, this, std::placeholders::_1));
}

void Sandbox::onStartup()
{
	initScene();
	addEventCallback(std::bind(&Sandbox::onEvent, this, std::placeholders::_1));
}

void Sandbox::onUpdate(float deltaTime)
{
	using namespace std::placeholders;
}

Application* createApplication()
{
	return new Sandbox();
}
