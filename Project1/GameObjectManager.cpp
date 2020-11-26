#include "GameObjectManager.h"
#include "Renderer.h"

template <> GameObjectManager* Singleton<GameObjectManager>::msSingleton = 0;

GameObjectManager* GameObjectManager::getSingletonPtr()
{
	return msSingleton;
}

GameObjectManager& GameObjectManager::getSingleton()
{
	assert(msSingleton); return (*msSingleton);
}

GameObjectManager::GameObjectManager() 
{
	printf("Game object manager constructed");
}

GameObjectManager::~GameObjectManager() 
{
	printf("game object manager deleted");
}

void GameObjectManager::setCameraId(int id)
{
	assert(id < m_cameras.size());
	m_currentCameraId = id;
}

int GameObjectManager::addGameObject(GameObject* gameObject)
{
	m_gameObjects.push_back(gameObject);
	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), m_pipeline);
	return static_cast<int>(m_gameObjects.size()-1);
}

int GameObjectManager::addGameObject(GameObject* gameObject, GraphicsPipeline* pipeline)
{
	m_gameObjects.push_back(gameObject);
	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
	return static_cast<int>(m_gameObjects.size()-1);
}


int GameObjectManager::addCamera(BaseCamera* camera)
{
	m_cameras.push_back(camera);
	return static_cast<int>(m_cameras.size()-1);
}

GameObject* GameObjectManager::getGameObjectPtrById(int id)
{
	assert(id < m_gameObjects.size() && id >= 0);
	return m_gameObjects[id];
}

void GameObjectManager::updateViewMatricies()
{
	glm::mat4 currentView = m_cameras[m_currentCameraId]->getView();
	for (GameObject* object : m_gameObjects)
	{
		object->getMeshPtr()->setViewMatrix(currentView);
	}
}
