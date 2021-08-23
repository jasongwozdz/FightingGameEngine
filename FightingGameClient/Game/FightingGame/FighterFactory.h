#pragma once
#include <Scene/Scene.h>
#include "Fighter.h"
#include "InputHandler.h"

class FighterFactory
{
public:
	FighterFactory(Scene& scene);

	virtual bool populateAttackInput(std::string fileLocation, Fighter* fighter);

	virtual bool populateAttacks(std::string fileLocation, Fighter* fighter);

	Fighter* createFighter(const std::string& modelPath,const std::string& texturePath, InputHandler& inputHandler);

	Fighter* createFighter(const std::string& fighterFilePath, InputHandler& inputHandler);
private:
	Scene& scene_;
};
