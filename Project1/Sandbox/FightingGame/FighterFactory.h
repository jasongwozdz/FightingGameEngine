#pragma once
#include <Scene/Scene.h>
#include "Fighter.h"
#include "InputHandler.h"

class FighterFactory
{
public:
	FighterFactory(Scene& scene, InputHandler& inputHandler);

	Fighter* createFighter(const std::string& modelPath,const std::string& texturePath);
private:
	Scene& scene_;
	InputHandler& inputHandler_;
};
