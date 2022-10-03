#pragma once
#include "../Entity.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera
{
	Camera(Entity* entity) :
		entity_(entity),
		projection(PERSPECTIVE),
		viewType(ViewType::LOOKAT)
	{
		projectionData.perspectiveData.fovAngleInDegrees = 45.0f;
		projectionData.perspectiveData.nearView = 0.5f;
		projectionData.perspectiveData.farView = 100.0f;
	}

	struct PerspectiveData
	{
		float fovAngleInDegrees;
		float nearView;
		float farView;
	};

	struct OrthographicData
	{
		float nearView;
		float farView;
		float left;
		float right;
		float up;
		float down;
	};

	union
	{
		PerspectiveData perspectiveData;
		OrthographicData orthographicData;
	} projectionData;

	enum Projection
	{
		PERSPECTIVE = 0,
		ORTHOGRAPHIC 
	} projection;

	enum class ViewType
	{
		LOOKAT,
		ARCBALL
	} viewType;

	struct ArcBallData
	{
		glm::quat rotation;
		glm::vec3 posToLookAt;
		float distance;
	};

	struct
	{
		ArcBallData arcBallData;
	} viewTypeData;

	Entity* entity_;
};

namespace CameraUtils
{
void calculateCameraView(const Camera* camera, glm::mat4* outViewMatrix);
void calculateProjectionMatrix(const Camera* camera, glm::mat4* outMatrix);
void handleMouseInput(const Camera* camera, glm::vec2 mousePosNew, glm::vec2 mousePosOld);
};
