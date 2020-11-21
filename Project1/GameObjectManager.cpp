#include "GameObjectManager.h"
#include "Renderer.h"


GameObjectManager::GameObjectManager() 
{
	printf("inside game object manager constructor");
}

GameObjectManager::~GameObjectManager() 
{
	printf("game object manager deleted");
}

int GameObjectManager::addGameObject(GameObject* gameObject)
{
	gameObjects.push_back(gameObject);
	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
	printf("Graphics pipeline : %p", pipeline);
	return gameObjects.size()-1;
}

int GameObjectManager::addCamera(BaseCamera* camera)
{
	cameras.push_back(camera);
	return cameras.size()-1;
}

GameObject* GameObjectManager::getGameObjectPtrById(int id)
{
	assert(id < gameObjects.size() && id >= 0);
	return gameObjects[id];
}

void GameObjectManager::updateViewMatricies()
{
	glm::mat4 currentView = cameras[currentCameraId]->getView();
	for (GameObject* object : gameObjects)
	{
		object->getMeshPtr()->setViewMatrix(currentView);
	}
}
