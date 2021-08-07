#pragma once
#include "Application.h"

class AnimatorTool : public Application
{
public:
	AnimatorTool() = default;
	~AnimatorTool();
	virtual void onUpdate(float deltaTime);
	virtual void onStartup();
};
