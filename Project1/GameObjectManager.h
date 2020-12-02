#pragma once
#include <vector>
#include "GameObject.h"
#include "BaseCamera.h"
#include "Singleton.h"
#include "PipelineManager.h"

class GameObjectManager : Singleton<GameObjectManager>
{
public:
	GameObjectManager();
	~GameObjectManager();
	int addGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type); //return GameObject id
	int addGameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, PipelineTypes type);
	int addCamera(BaseCamera* camera);
	void setCameraId(int id);
	GameObject* getGameObjectPtrById(int id);
	void updateViewMatricies();
	void updateGameObjectPosition(int id, glm::vec3 pos);

	static GameObjectManager* getSingletonPtr();
	static GameObjectManager& getSingleton();
private:
	std::vector<GameObject*> m_gameObjects;
	std::vector<BaseCamera*> m_cameras;
	int m_currentCameraId = 0;
};

