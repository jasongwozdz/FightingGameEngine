#pragma once
#include "Application.h"

int main(int argc, char** argv)
{
	Application* app = createApplication();
	app->run();
	delete app;
	return 1;
}
