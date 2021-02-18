#include "GameObjectManager.h"
#include "Renderer/Renderer.h"

//template <> GameObjectManager* Singleton<GameObjectManager>::msSingleton = 0;
//
//GameObjectManager* GameObjectManager::getSingletonPtr()
//{
//	return msSingleton;
//}
//
//GameObjectManager& GameObjectManager::getSingleton()
//{
//	assert(msSingleton); return (*msSingleton);
//}
//
//GameObjectManager::~GameObjectManager() 
//{
//	std::cout << "delete GameObjectManager" << std::endl;
//	for (auto gObject : m_gameObjects)
//	{
//		delete gObject;
//	}
//	auto it = m_animators.begin();
//	while (it != m_animators.end())
//	{
//		delete it->second;
//		it++;
//	}
//}
//
//void GameObjectManager::setCameraId(int id)
//{
//	assert(id < m_cameras.size());
//	m_currentCameraId = id;
//}
//
//int GameObjectManager::addGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type)
//{
//	VkDescriptorSetLayout layout;
//	GraphicsPipeline* pipeline = PipelineManager::getSingleton().createOrGetPipeline(type, layout);
//	
//	GameObject* gameObject = new GameObject(modelPath, texturePath, pos, layout);
//	gameObject->setPosition(pos);
//	m_gameObjects.push_back(gameObject);
//	
//	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
//	return static_cast<int>(m_gameObjects.size()-1);
//}
//
//int GameObjectManager::addGameObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 pos, PipelineTypes type)
//{
//	VkDescriptorSetLayout layout;
//	GraphicsPipeline* pipeline = PipelineManager::getSingleton().createOrGetPipeline(type, layout);
//	
//	GameObject* gameObject = new GameObject(vertices, indices, pos, layout);
//	gameObject->setPosition(pos);
//	m_gameObjects.push_back(gameObject);
//	
//	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
//	return static_cast<int>(m_gameObjects.size()-1);
//}
//
//int GameObjectManager::addAnimatedGameObject(std::string modelPath, std::string texturePath, glm::vec3 pos, PipelineTypes type)
//{
//	VkDescriptorSetLayout layout;
//	GraphicsPipeline* pipeline = PipelineManager::getSingleton().createOrGetPipeline(type, layout);
//
//	AnimationReturnVals vals = ResourceManager::getSingleton().loadAnimationFile(modelPath);
//	
//	AnimatedGameObject* gameObject = new AnimatedGameObject(vals.vertices, vals.indices, pos, layout, 0, vals.scene, vals.boneMapping, vals.boneInfo);
//	gameObject->setPosition(pos);
//	m_gameObjects.push_back(gameObject);
//	int id = static_cast<int>(m_gameObjects.size() - 1);
//	
//	Animator* animator = new Animator(*gameObject);
//
//	m_animators[id] = animator;
//
//	Renderer::getSingleton().bindTexturedMeshToPipeline(gameObject->getMeshPtr(), pipeline);
//	return id;
//}
//
//int GameObjectManager::addCamera(BaseCamera* camera)
//{
//	m_cameras.push_back(camera);
//	m_currentCameraId = static_cast<int>(m_cameras.size() - 1);
//	return m_currentCameraId;
//}
//
//GameObject* GameObjectManager::getGameObjectPtrById(int id)
//{
//	assert(id < m_gameObjects.size() && id >= 0);
//	return m_gameObjects[id];
//}
//
//void GameObjectManager::update(float deltaTime)
//{
//	updateViewMatricies();
//	upateAnimatedObjects(deltaTime);
//}
//
//void GameObjectManager::updateViewMatricies()
//{
//	glm::mat4 currentView;
//	if (m_currentCameraId != -1)
//	{
//		currentView = m_cameras[m_currentCameraId]->getView();
//	}
//	for (GameObject* object : m_gameObjects)
//	{
//		object->getMeshPtr()->setViewMatrix(currentView);
//	}
//}
//
//void GameObjectManager::upateAnimatedObjects(float deltaTime)
//{
//	for (auto anim : m_animators)
//	{
//		anim.second->update(deltaTime);
//	}
//}
//
//void GameObjectManager::updateGameObjectPosition(int id, glm::vec3 pos)
//{
//	assert(id < m_gameObjects.size());
//	GameObject* g = getGameObjectPtrById(id);
//	g->setPosition(pos);
//}
