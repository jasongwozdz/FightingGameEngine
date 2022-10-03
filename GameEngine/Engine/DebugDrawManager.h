#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "Vertex.h"
#include "BaseCamera.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "Scene/Scene.h"
#include "Singleton.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API DebugDrawManager : public Singleton<DebugDrawManager>
{
public:
	DebugDrawManager();
	~DebugDrawManager() = default;
	static DebugDrawManager& getSingleton();
	static DebugDrawManager* getSingletonPtr();

	virtual void drawLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color);
	virtual void drawRect(glm::vec3 pos, glm::vec3 color, float minX, float maxX, float minY, float maxY);
	virtual void drawCube(glm::vec3 pos, glm::vec3 size /*L x W x H*/, glm::vec3 color);
	virtual void drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color);

	//PickerPipelipeline Methods
	virtual void drawFilledRect(glm::vec3 pos, const glm::vec3& color, glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, const int uniqueId);
	virtual bool getSelectedObject(int& id);
};
