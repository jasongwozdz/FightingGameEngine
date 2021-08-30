#include "BaseCamera.h"
#include <iostream>
#include <cstdio>
#include <ctime>
#include "libs/imgui/imgui.h"
#include "libs/imgui/implot/implot.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "EngineSettings.h"

BaseCamera::BaseCamera(glm::vec3 pos, glm::vec3 direction, glm::vec3 upDir)
{
	position = pos;
	viewDirection = direction;
	upDirection = upDir;
	projectionMatrix = glm::perspective(glm::radians(45.0f), EngineSettings::getSingleton().windowWidth / (float)EngineSettings::getSingleton().windowHeight, 0.1f, 100.0f);
	projectionMatrix[1][1] *= -1;//need to flip this because GLM uses OpenGl coordinates where top left is -1,1 where as in vulkan top left is -1,-1.  Flip y scale
}

BaseCamera::~BaseCamera() {};

void BaseCamera::setOldMousePosition(glm::vec2 oldMouse)
{
	oldMousePosition = oldMouse;
}

void BaseCamera::updateMouse(glm::vec2 newMousePosition) 
{
	glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

	//rotate around up axis by delta x 
	viewDirection = glm::mat3(glm::rotate(-mouseDelta.x * rotationSpeed, upDirection)) * viewDirection;
	
	//get new axis to rotate around for y direction
	glm::vec3 toRotateAround = glm::cross(viewDirection, upDirection);

	//rotate around y axis by delta y
	viewDirection = glm::mat3(glm::rotate(-mouseDelta.y * rotationSpeed, toRotateAround)) * viewDirection; 
	oldMousePosition = newMousePosition;
}

void BaseCamera::resetProjectionMatrix(const float width, const float height)
{
	if (width == 0 || height == 0)
	{
		return;
	}
	projectionMatrix = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
	projectionMatrix[1][1] *= -1;//need to flip this because GLM uses OpenGl coordinates where top left is -1,1 where as in vulkan top left is -1,-1.  Flip y scale
}

void BaseCamera::moveForward(float time) 
{
	position += time * cameraSpeed * viewDirection;
}

void BaseCamera::moveBackward(float time) 
{
	position -= time * cameraSpeed * viewDirection;
}

void BaseCamera::strafeLeft(float time) 
{
	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
	position -= time * cameraSpeed * strafeDirection;
}

void BaseCamera::strafeRight(float time) 
{
	glm::vec3 strafeDirection = glm::cross(viewDirection, upDirection);
	position += time * cameraSpeed * strafeDirection;
}

glm::mat4 const BaseCamera::getView() 
{
	glm::mat4 view = glm::lookAt(position, position + viewDirection, upDirection);
	return view;
}

void BaseCamera::update(float deltaTime)
{
	if (drawDebug_)
	{
		float time = std::clock();
		times_.insert(times_.begin(), time);
		viewDirectionsX_.insert(viewDirectionsX_.begin(), viewDirection.x);
		viewDirectionsY_.insert(viewDirectionsY_.begin(), viewDirection.y);
		viewDirectionsZ_.insert(viewDirectionsZ_.begin(), viewDirection.z);

		posX_.insert(posX_.begin(), position.x);
		posY_.insert(posY_.begin(), position.y);
		posZ_.insert(posZ_.begin(), position.z);
		if (times_.size() > numberOfDataPoints_)
		{
			times_.resize(numberOfDataPoints_);
			viewDirectionsX_.resize(numberOfDataPoints_);
			viewDirectionsY_.resize(numberOfDataPoints_);
			viewDirectionsZ_.resize(numberOfDataPoints_);
			posX_.resize(numberOfDataPoints_);
			posY_.resize(numberOfDataPoints_);
			posZ_.resize(numberOfDataPoints_);

		}

		ImGui::Begin("Camera Debug");
		char buffer[500];
		sprintf_s(buffer, "Position: %f, %f, %f", position.x, position.y, position.z);
		ImGui::Text(buffer);
		sprintf_s(buffer, "ViewDirection: %f, %f, %f", viewDirection.x, viewDirection.y, viewDirection.z);
		ImGui::Text(buffer);
		sprintf_s(buffer, "upDirection: %f, %f, %f", upDirection.x, upDirection.y, viewDirection.z);
		ImGui::Text(buffer);

		ImPlot::SetNextPlotLimitsX(time - 3000.0f, time, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(-100, 100);
		if (ImPlot::BeginPlot("viewDirections", "time", "viewDirection"))
		{
			ImPlot::PlotLine("View Direction X", times_.data(), viewDirectionsX_.data(), viewDirectionsX_.size());
			ImPlot::PlotLine("View Direction Y", times_.data(), viewDirectionsY_.data(), viewDirectionsY_.size());
			ImPlot::PlotLine("View Direction Z", times_.data(), viewDirectionsZ_.data(), viewDirectionsZ_.size());

			ImPlot::EndPlot();
		}

		ImPlot::SetNextPlotLimitsX(time - 3000.0f, time, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(-100, 100);
		if (ImPlot::BeginPlot("Position", "time", "viewDirection"))
		{
			ImPlot::PlotLine("Position X", times_.data(), posX_.data(), posX_.size());
			ImPlot::PlotLine("Position Y", times_.data(), posY_.data(), posY_.size());
			ImPlot::PlotLine("Position Z", times_.data(), posZ_.data(), posZ_.size());

			ImPlot::EndPlot();
		}
		ImGui::End();
	}
}

