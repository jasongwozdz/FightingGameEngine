#include "DebugDrawManager.h"

template<> DebugDrawManager* Singleton<DebugDrawManager>::msSingleton = 0;


DebugDrawManager::DebugDrawManager() : 
	Singleton<DebugDrawManager>()
{
	msSingleton = this;
}

DebugDrawManager& DebugDrawManager::getSingleton()
{
	_ASSERTE(msSingleton, ("Need to initalize debugDrawManager"));
	return *msSingleton;
}

DebugDrawManager* DebugDrawManager::getSingletonPtr()
{
	_ASSERTE(msSingleton, ("Need to initalize debugDrawManager"));
	return msSingleton;
}


void DebugDrawManager::drawLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color)
{}
void DebugDrawManager::drawRect(glm::vec3 pos, glm::vec3 color, float minX, float maxX, float minY, float maxY)
{}
void DebugDrawManager::drawCube(glm::vec3 pos, glm::vec3 size /*L x W x H*/, glm::vec3 color)
{}
void DebugDrawManager::drawGrid(const glm::vec3& position, const glm::vec3& axisOfRotation, const float rotationAngle, const int width, const int height, const glm::vec3& color)
{}

//PickerPipelipeline Methods
void DebugDrawManager::drawFilledRect(glm::vec3 pos, const glm::vec3& color, glm::vec3& axisOfRotation, const float rotationAngle, const float minX, const float maxX, const float minY, const float maxY, const int uniqueId) {}

bool DebugDrawManager::getSelectedObject(int& id) { return false; };
