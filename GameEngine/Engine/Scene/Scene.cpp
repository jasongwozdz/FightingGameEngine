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
		drawCameraFrustrum(camera, transform);
	}

	auto behaviorView = registry_.view<Behavior>();
	for (auto entity : behaviorView)
	{
		auto& behavior = behaviorView.get<Behavior>(entity);
		behavior.update();
	}

	boxCollisionManager_->update(this);

	calculateViewProjection(view, projection);
	auto v = registry_.view<Transform>();
	for (auto entity : v)
	{
		auto& transform = v.get<Transform>(entity);
		glm::mat4 finalTransform = transform.calculateTransform();
		transform.drawDebug();
		
		Renderable* mesh = registry_.try_get<Renderable>(entity);
		if (mesh)
		{
			transform.applyTransformToMesh(*mesh);
			mesh->ubo_.view = view;
			mesh->ubo_.proj = projection;

			Animator* animator = registry_.try_get<Animator>(entity);
			if (animator && mesh->render_)
			{
				animator->update(deltaTime, *mesh);
			}

			if (!mesh->uploaded_)
			{
				Textured* texture = registry_.try_get<Textured>(entity);
				if (texture != 0)
					renderer_->uploadObject(mesh, texture, animator);
				else
					renderer_->uploadObject(mesh);

				mesh->uploaded_ = true;
			}

			if (mesh->render_)
			{
				objectsToDraw_.push_back(mesh);
			}
		}
	}
	renderer_->draw(objectsToDraw_);
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

std::vector<Entity*> Scene::getEntities(std::string entityName)
{
	auto entry = entityNameMap_.find(entityName);
	if (entry != entityNameMap_.end())
	{
		return entry->second;
	}
	else
	{
		std::cout << "ERROR couln't find an entity with that name" << std::endl;
	}
	return std::vector<Entity*>();
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

	viewMatrix = glm::lookAt(transform.position_, transform.position_ + transform.forward(), transform.up());

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
