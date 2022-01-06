#pragma once
#include "../Application.h"

class Editor : public Application
{
public:
	Editor() = default;
	~Editor() = default;

	void onUpdate(float deltaTime);
	void onStartup();
	void onEvent(Events::Event& event);

	void moveCamera(Events::KeyEvent& keyEvent);
	void adjustCameraRotation();

private:

	float deltaTime_;
};
