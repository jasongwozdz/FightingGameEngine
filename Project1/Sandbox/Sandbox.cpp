#include <iostream>
#include "Sandbox.h"
#include "../EntryPoint.h"

Sandbox::~Sandbox()
{
	delete baseCamera;
}

void Sandbox::initScene()
{
	Entity& e = scene_->addEntity("Temp");

	Transform& t = e.addComponent<Transform>( 0.0f,0.0f,0.0f );

	std::string modelPath = "./Models/viking_room.obj";
	std::string texturePath = "./Textures/viking_room.png";
	ModelReturnVals vals = resourceManager->loadObjFile(modelPath);
	TextureReturnVals texVals = resourceManager->loadTextureFile(texturePath);
	Renderable& r = e.addComponent<Renderable>(vals.vertices, vals.indices, true, "Viking_Room");
	Textured& tex = e.addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Viking_Room");

	baseCamera = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, {0.0f, 0.0f, 1.0f});
	scene_->addCamera(baseCamera);
	//debugDrawManager->drawGrid({ 255, 255, 255 }, true);
}

void Sandbox::handleMouseClick(Events::MousePressedEvent& e)
{
	std::cout << "Mouse clicked" << std::endl;
}

void Sandbox::handleKeyButtonDown(Events::KeyPressedEvent& e)
{
	std::cout << "KeyCode: " << e.KeyCode << std::endl;
	switch (e.KeyCode)
	{
	case 87: // w
		baseCamera->moveForward(1);
		break;

	case 83: // s
		baseCamera->moveBackward(1);
		break;

	case 65: // a
		baseCamera->strafeLeft(1);
		break;

	case 68: // d
		baseCamera->strafeRight(1);
		break;
	}
}

void Sandbox::handleMouseMove(Events::MouseMoveEvent& e)
{
	//baseCamera->updateMouse({ e.mouseX, e.mouseY });
}

void Sandbox::onEvent(Events::Event& e)
{
	Events::EventDispatcher d(e);
	d.dispatch<Events::MousePressedEvent>(std::bind(&Sandbox::handleMouseClick, this, std::placeholders::_1));
	d.dispatch<Events::MouseMoveEvent>(std::bind(&Sandbox::handleMouseMove, this, std::placeholders::_1));
	d.dispatch<Events::KeyPressedEvent>(std::bind(&Sandbox::handleKeyButtonDown, this, std::placeholders::_1));
}

void Sandbox::onStartup()
{
	initScene();
	addEventCallback(std::bind(&Sandbox::onEvent, this, std::placeholders::_1));
}

void eachTransform(entt::entity e, entt::registry r, Transform t)
{

}

void Sandbox::onUpdate(float deltaTime)
{
	//scene_->each<Transform>([=](entt::entity e, entt::registry r, Transform t) {
	//	
	//});

	//scene_->each<Transform>(eachTransform);
}

Application* createApplication()
{
	return new Sandbox();
}
