#pragma once
#include "../Entity.h"

struct Camera
{
	Camera(Entity* entity) :
		entity_(entity),
		fovAngleInDegrees_(45.0f),
		nearView(0.5f),
		farView(100.0f),
		projection(PERSPECTIVE)
	{}

	Entity* entity_;
	float fovAngleInDegrees_;
	float nearView;
	float farView;
	enum Projection
	{
		PERSPECTIVE = 0,
		ORTHOGRAPHIC 
	} projection;
};
