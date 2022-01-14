#include "Editor.h"

#include <iostream>
#include <vector>

#include "../EntryPoint.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Behavior.h"
#include "Scene/Behaviors/ControllableCameraBehavior.h"
#include "Renderer/UIInterface.h"
#include "Input.h"

Application* createApplication()
{
	return new Editor();
}

void Editor::onStartup()
{
	setCursor(true);
	Entity* currentCamera = scene_->getCurrentCamera().entity_;
	currentCamera->addComponent<Behavior>(new ControllableCameraBehavior(currentCamera));
	input_->addButton("Escape", GLFW_KEY_ESCAPE);
}

bool Editor::addEntity(std::string path, std::string name)
{
	AnimationReturnVals mesh = resourceManager_->loadAnimationFile(path);
	if (!mesh.succesful)
	{
		return false;
	}
	Entity* entity = scene_->addEntity(name);
	entity->addComponent<Transform>(0, 0, 0);
	entity->addComponent<Renderable>(mesh.vertices, mesh.indices, true, name, true);
	entitys_.push_back(entity);
	currentlySelected_ = entity;
	return true;
}

void Editor::drawUI()
{
	{
		static bool clickedOn = false;
		std::string pathToMesh;

		uiInterface_->beginMainMenuBar();
		uiInterface_->addMenuItem("Add Entiy", &clickedOn);
		uiInterface_->endMainMenuBar();

		if (clickedOn)
		{
			static bool addingPath = false;
			uiInterface_->beginWindow("Add New Entity", engineSettings_->windowWidth / 5, engineSettings_->windowHeight / 5, { engineSettings_->windowWidth / 2, engineSettings_->windowHeight / 2 }, nullptr, false);
			if (uiInterface_->addButton("Add Cube"))
			{
				Entity* cube = scene_->addEntity("Cube");
				ModelReturnVals mesh = resourceManager_->loadObjFile(cubeModelPath_);
				cube->addComponent<Renderable>(mesh.vertices, mesh.indices, true, "Cube", true);
				Transform& transform = cube->addComponent<Transform>(0.0f, 0.0f, 0.0f);
				entitys_.push_back(cube);
				currentlySelected_ = cube;
				clickedOn = false;
			}
			if (uiInterface_->addButton("Add Custom Mesh"))
			{
				addingPath = true;

			}
			if (addingPath)
			{
				uiInterface_->beginWindow("Path to entity", engineSettings_->windowWidth / 5, engineSettings_->windowHeight / 5, { engineSettings_->windowWidth / 2, engineSettings_->windowHeight / 2 }, nullptr, false);
				if (uiInterface_->addInput("Path to mesh", &pathToMesh))
				{
					addEntity(pathToMesh, "NewEnt");
					addingPath = false;
					clickedOn = false;
				}
				uiInterface_->EndWindow();
			}
			uiInterface_->EndWindow();
		}
	}
	
	{
		uiInterface_->beginWindow("Entities", engineSettings_->windowWidth / 4, engineSettings_->windowHeight/2, {engineSettings_->windowWidth/8, engineSettings_->windowHeight/2}, nullptr, false);
		for (int i = 0; i < entitys_.size(); i++)
		{
			bool clickedOn = false;
			uiInterface_->addMenuItem(entitys_[i]->name_, &clickedOn);
			if (clickedOn)
			{
				currentlySelected_ = entitys_[i];
			}
		}
		uiInterface_->EndWindow();
	}

	if (currentlySelected_)
	{
		Transform& transform = currentlySelected_->getComponent<Transform>();
		float x = transform.position_.x;
		float y = transform.position_.y;
		float z = transform.position_.z;
		float scale = transform.scale_.x;

		uiInterface_->beginWindow("Entity Components", engineSettings_->windowHeight / 5, engineSettings_->windowHeight / 5, { engineSettings_->windowWidth - (engineSettings_->windowWidth / 5), engineSettings_->windowHeight / 2 });
		uiInterface_->addInputFloat("X: ", x, transform.position_.x);
		uiInterface_->addInputFloat("Y: ", y, transform.position_.y);
		uiInterface_->addInputFloat("Z: ", z, transform.position_.z);

		uiInterface_->addInputFloat("Scale: ", scale, 1.0f);

		transform.position_ = { x, y, z };
		transform.scale_ = { scale, scale, scale };

		uiInterface_->EndWindow();
	}
}

void Editor::handleInput()
{
	if (input_->getButton("Escape") > 0.0f)
	{
		currentlySelected_ = nullptr;
	}
}

void Editor::onUpdate(float deltaTime)
{
	deltaTime_ = deltaTime;
	glm::vec3 origin = { 0, 0, 1 };
	debugManager_->drawLine(origin, origin + Transform::worldForward, { 0,0,255 });//forward blue
	debugManager_->drawLine(origin, origin + Transform::worldLeft, { 255, 0, 0 });//right red
	debugManager_->drawLine(origin, origin + Transform::worldUp, { 0, 255,  0});//blue up
	debugManager_->drawGrid({ 0, 0, 0 }, Transform::worldLeft, 90.0f, 25, 25, { 255, 255, 255 });
	drawUI();
	handleInput();
}
