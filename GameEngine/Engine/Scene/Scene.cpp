#include "Scene.h"
#include "../Renderer/SkyBoxRenderSubsystem.h"
#include "Components/Behavior.h"
#include "Components/Camera.h"
#include "../../Engine/EngineSettings.h"
#include "../DebugDrawManager.h"
#include "../BoxCollisionManager.h"

template<> Scene* Singleton<Scene>::msSingleton = 0;
float Scene::DeltaTime = 0.0f;
float Scene::DeltaTimeMs = 0.0f;

Scene* Scene::getSingletonPtr()
{
	if (msSingleton == 0)
	{
		msSingleton = new Scene();
	}
	return msSingleton;
}

void Scene::clearScene()
{
	auto transform = registry_.view<Transform>();
	for (auto entity : transform)
	{
		delete entitys_[entity];
	}
	entityNameMap_.clear();
	registry_.clear();
	initalizeDefaultCamera();
}

Scene& Scene::getSingleton()
{
	if (msSingleton == 0)
	{
		msSingleton = new Scene();
	}
	assert(msSingleton); return (*msSingleton);
}

Scene::Scene() : 
	renderer_(VkRenderer::getSingletonPtr()),
	boxCollisionManager_(new BoxCollisionManager())
{
	skybox_ = renderer_->addRenderSubsystem<SkyBoxRenderSubsystem>();
	initalizeDefaultCamera();
}

Scene::~Scene()
{
	delete boxCollisionManager_;
	registry_.clear();
}

void Scene::update(float deltaTime)
{
	DeltaTime = deltaTime;
	DeltaTimeMs = deltaTime * 0.001f;
	glm::mat4 view, projection;

	auto cameraView = registry_.view<Camera, Transform>();
	for(auto entity : cameraView)
	{
		Camera& camera = cameraView.get<Camera>(entity);
		Transform& transform = cameraView.get<Transform>(entity);
		//drawCameraFrustrum(camera, transform);
	}

	auto behaviorView = registry_.view<Behavior>();
	for (auto entity : behaviorView)
	{
		auto& behavior = behaviorView.get<Behavior>(entity);
		behavior.update();
	}

	boxCollisionManager_->update(this);

	calculateViewProjection(view, projection);

	auto assetInstanceView = registry_.view<AssetInstance, Transform>();
	std::vector<AssetInstance*> assetInstancesToDraw;
	for (auto entity : assetInstanceView)
	{
		AssetInstance& assetInstance = assetInstanceView.get<AssetInstance>(entity);
		Transform& transform = assetInstanceView.get<Transform>(entity);
		assetInstance.setModelMatrix(transform.calculateTransform());
		assetInstance.setViewMatrix(view);
		assetInstance.setProjectionMatrix(projection);
		assetInstancesToDraw.push_back(&assetInstance);
		Animator* animator = registry_.try_get<Animator>(entity);
		if (animator)
		{
			animator->update(deltaTime, &assetInstance);
		}
	}

	auto lightSourceInstanceView = registry_.view<DirLight>();
	DirLight dirLight;
	for(auto entity : lightSourceInstanceView)
	{
		dirLight = lightSourceInstanceView.get<DirLight>(entity);
	}

	std::vector<PointLight> pointLights;
	auto pointLightInstanceView = registry_.view<PointLight, Transform>();
	for (auto entity : pointLightInstanceView)
	{
		PointLight& pointLight = pointLightInstanceView.get<PointLight>(entity);
		Transform& transform = pointLightInstanceView.get<Transform>(entity);
		transform.drawDebug();
		transform.calculateTransform();
		pointLight.uniformData_.position = transform.position_;
		pointLights.push_back(pointLight);
	}

	renderer_->draw(objectsToDraw_, assetInstancesToDraw, dirLight, pointLights);
	objectsToDraw_.clear();
}

Entity* Scene::addEntity(std::string name)
{
	Entity* entity = new Entity(registry_, name);
	entitys_.insert({ entity->enttId_, entity });
	
	auto entry = entityNameMap_.find(name);
	if (entry == entityNameMap_.end())
	{
		entityNameMap_.insert({ name, std::vector<Entity*>(1, entity) });
	}
	else
	{
		entityNameMap_[name].push_back(entity);
	}
	return entity;
}

void Scene::getEntities(std::string entityName, std::vector<Entity*>& outEnt)
{
	auto entry = entityNameMap_.find(entityName);
	if (entry != entityNameMap_.end())
	{
		outEnt = entry->second;
	}
	else
	{
		std::cout << "ERROR couln't find an entity with that name" << std::endl;
	}
}

bool Scene::setSkybox(const std::string& path)
{
	return(skybox_->setSkyboxTexture(path));
}

void Scene::setActiveCamera(Entity* entity)
{
	Camera* camera = entity->tryGetComponent<Camera>();
	assert(camera);
	activeCamera_ = entity;
}

void Scene::drawCameraFrustrum(Camera& currentCamera, Transform& cameraTransform)
{
	DebugDrawManager& debugDrawManager = DebugDrawManager::getSingleton();
	//debugDrawManager.drawCube(cameraTransform.position_, { 1.0f, 1.0f, 1.0f }, { 2.0f , 1.0f, 1.0f });
	float angleRads = glm::radians(currentCamera.fovAngleInDegrees_);
	glm::vec3 dirRight = glm::vec3(glm::sin(angleRads), 0.0f, glm::cos(angleRads));
	glm::vec3 dirLeft = glm::vec3(glm::sin(-angleRads), 0.0f, glm::cos(-angleRads));
	glm::vec3 worldDirRight = cameraTransform.calculateTransform() * glm::vec4(dirRight, 0.0f);
	glm::vec3 worldDirLeft = cameraTransform.calculateTransform() * glm::vec4(dirLeft, 0.0f);
	debugDrawManager.drawLine(cameraTransform.position_, cameraTransform.position_ + worldDirRight * 3.0f, { 255, 255, 255 });
	debugDrawManager.drawLine(cameraTransform.position_, cameraTransform.position_ + worldDirLeft * 3.0f, { 255, 255, 255 });
}

void Scene::calculateViewProjection(glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	EngineSettings& engineSettings = EngineSettings::getSingleton();
	float width = EngineSettings::getSingleton().windowWidth;
	float height = EngineSettings::getSingleton().windowHeight;
	float aspectRatio = width / height;

	Camera& camera = activeCamera_->getComponent<Camera>();
	Transform& transform = activeCamera_->getComponent<Transform>();

	viewMatrix = glm::lookAt(transform.position_, transform.position_ + transform.forward(), Transform::worldUp);

	if (camera.projection == Camera::Projection::ORTHOGRAPHIC)
	{
		projectionMatrix = glm::ortho(0.0f, width, 0.0f, height, camera.nearView, camera.farView);
	}
	else
	{
		projectionMatrix = glm::perspective(glm::radians(camera.fovAngleInDegrees_), aspectRatio, camera.nearView, camera.farView);
	}
	projectionMatrix[1][1] *= -1;//need to flip this because GLM uses OpenGl coordinates where top left is -1,1 where as in vulkan top left is -1,-1.  Flip y scale
	//drawCameraFrustrum(camera, transform);
}

Entity * Scene::createCameraEntity()
{
	Entity* camera = addEntity("Camera");
	camera->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	camera->addComponent<Camera>(camera);
	return camera;
}

void Scene::initalizeDefaultCamera()
{
	activeCamera_ = addEntity("Camera");
	activeCamera_->addComponent<Transform>(Transform(glm::vec3(0.0f, 0.0f, 0.0)));
	activeCamera_->addComponent<Camera>(activeCamera_);
}

Camera& Scene::getCurrentCamera()
{
	return activeCamera_->getComponent<Camera>();
}
