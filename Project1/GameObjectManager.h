#pragma once
#include <vector>
#include "GameObject.h"
#include "BaseCamera.h"

class GameObjectManager
{
public:
	GameObjectManager();
	~GameObjectManager();
	int addGameObject(GameObject* gameObject); //return GameObject id
	int addCamera(BaseCamera* camera);
	void setCameraId(int id);
	GameObject* getGameObjectPtrById(int id);
	void updateViewMatricies();
private:
	std::vector<GameObject*> gameObjects;
	std::vector<BaseCamera*> cameras;
	int currentCameraId = 0;
	GraphicsPipeline* pipeline = nullptr;
};

