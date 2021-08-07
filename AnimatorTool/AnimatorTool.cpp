#include <iostream>
#include "AnimatorTool.h"
#include "EntryPoint.h"

void AnimatorTool::onStartup()
{
	std::cout << "hello" << std::endl;
}

void AnimatorTool::onUpdate(float deltaTime)
{

	std::cout << "hello" << std::endl;
}

AnimatorTool::~AnimatorTool()
{

}

Application* createApplication()
{
	return new AnimatorTool();
}

