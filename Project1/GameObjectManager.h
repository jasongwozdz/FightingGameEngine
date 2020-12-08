#pragma once
#include <vector>
#include "BaseCamera.h"
#include "Singleton.h"
#include "PipelineManager.h"
#include "AnimatedGameObject.h"
#include "Animator.h"

class GameObjectManager : Singleton<GameObjectManager>
{
public:
	GameObjectManager();
	~GameObjectManager();
	int addGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type); //return GameObject id
	int addGameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, PipelineTypes type);
	int addAnimatedGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type);
	int addCamera(BaseCamera* camera);
	void setCameraId(int id);
	GameObject* getGameObjectPtrById(int id);
	void updateViewMatricies();
	void updateGameObjectPosition(int id, glm::vec3 pos);
	void upateAnimatedObjects(float deltaTime);

	static GameObjectManager* getSingletonPtr();
	static GameObjectManager& getSingleton();
private:
	std::vector<GameObject*> m_gameObjects;
	std::map<int, Animator*> m_animators; //map object id to animator

	std::vector<BaseCamera*> m_cameras;
	int m_currentCameraId = 0;
};

