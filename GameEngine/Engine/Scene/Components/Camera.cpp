#include "Camera.h"
#include "Transform.h"
#include "../../EngineSettings.h"

void CameraUtils::calculateCameraView(const Camera* camera, glm::mat4* outViewMatrix)
{
	_ASSERT_EXPR(camera, "Camera is null, Can't calculate camera view.");
	_ASSERT_EXPR(outViewMatrix, "ViewMatrix is null, Can't calculate camera view.");
	switch (camera->viewType)
	{
		case Camera::ViewType::LOOKAT:
		{
			const Transform& transform = camera->entity_->getComponent<Transform>();
			*outViewMatrix = glm::lookAt(transform.position_, transform.position_ + transform.forward(), Transform::worldUp);
			break;
		}
		case Camera::ViewType::ARCBALL:
		{
			glm::vec3 distanceVec = { 0.0f, 0.0f, camera->viewTypeData.arcBallData.distance };
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), distanceVec);
			transform = transform * glm::toMat4(camera->viewTypeData.arcBallData.rotation);
			transform = glm::translate(transform, camera->viewTypeData.arcBallData.posToLookAt);
			break;
		}
		default:
		{
			_ASSERT_EXPR(false, "Camera view type not handeld");
			break;
		}
	}
}

void CameraUtils::calculateProjectionMatrix(const Camera* camera, glm::mat4* outMatrix)
{
	_ASSERT_EXPR(camera, ("Can't calulate projection matrix no camera"));
	_ASSERT_EXPR(outMatrix, ("Invalid out matrix"));
	if (camera->projection == Camera::Projection::PERSPECTIVE)
	{
		EngineSettings& engineSettings = EngineSettings::getSingleton();
		float width = EngineSettings::getSingleton().windowWidth;
		float height = EngineSettings::getSingleton().windowHeight;
		float aspectRatio = width / height;

		*outMatrix = glm::perspective(glm::radians(camera->projectionData.perspectiveData.fovAngleInDegrees), aspectRatio, camera->projectionData.perspectiveData.nearView, camera->projectionData.perspectiveData.farView);
	}
	else if(Camera::Projection::ORTHOGRAPHIC)
	{
		EngineSettings& engineSettings = EngineSettings::getSingleton();
		float width = EngineSettings::getSingleton().windowWidth;
		float height = EngineSettings::getSingleton().windowHeight;
		float aspectRatio = width / height;

		*outMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, camera->projectionData.orthographicData.nearView, camera->projectionData.orthographicData.farView);
	}
}
