#pragma once
#include "../Application.h"
#include "Scene/Entity.h"
#include "Renderer/UIInterface.h"

class Editor : public Application
{
public:
	Editor() = default;
	~Editor() = default;

	void onUpdate(float deltaTime);
	void onStartup();

private:
	[[nodiscard]] bool addEntity(std::string path, std::string name);
	void drawUI();
	void handleInput();

private:
	UI::UIInterface* uiInterface_;
	class Entity* currentlySelected_ = nullptr;
	std::vector<class Entity*> entitys_;
	const std::string cubeModelPath_ = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Models\\cube.obj";
	float deltaTime_;
};
