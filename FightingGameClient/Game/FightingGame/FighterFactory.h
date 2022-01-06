#pragma once
#include <Scene/Scene.h>
#include "Fighter/Fighter.h"
#include "InputHandler.h"

class FighterFactory
{
public:
	FighterFactory(Scene& scene);
	Entity* createFighter(const std::string& fighterFilePath, InputHandler& inputHandler);
private:
	virtual bool populateAttackInput(std::string fileLocation, Fighter* fighter);
private:
	Scene& scene_;
};
