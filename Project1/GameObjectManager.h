#pragma once
#include <vector>
#include "GameObject.h"
#include "BaseCamera.h"
#include "Singleton.h"

class GameObjectManager : Singleton<GameObjectManager>
{
public:
	GameObjectManager();
	~GameObjectManager();
	int addGameObject(GameObject* gameObject); //return GameObject id
	int addGameObject(GameObject* gameObject, GraphicsPipeline* pipeline);
	int addCamera(BaseCamera* camera);
	void setCameraId(int id);
	GameObject* getGameObjectPtrById(int id);
	void updateViewMatricies();
	static GameObjectManager* getSingletonPtr();
	static GameObjectManager& getSingleton();
private:
	std::vector<GameObject*> m_gameObjects;
	std::vector<BaseCamera*> m_cameras;
	int m_currentCameraId = 0;
	GraphicsPipeline* m_pipeline = nullptr;
};

