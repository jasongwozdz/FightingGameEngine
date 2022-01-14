#pragma once
#include <Scene/Scene.h>
#include "Fighter/Fighter.h"
#include "InputHandler.h"

class FighterFactory
{
public:
	FighterFactory(Scene& scene);
	Entity* createFighterNew(const std::string& fighterFilePath, InputHandler& inputHandler);
private:
	Scene& scene_;
};
