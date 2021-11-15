#include "Scene.h"
#include "../Renderer/SkyBoxRenderSubsystem.h"

template<> Scene* Singleton<Scene>::msSingleton = 0;

Scene* Scene::getSingletonPtr()
{
	if (msSingleton == 0)
	{
		msSingleton = new Scene();
	}
	return msSingleton;
}

Scene& Scene::getSingleton()
{
	if (msSingleton == 0)
	{
		msSingleton = new Scene();
	}
	assert(msSingleton); return (*msSingleton);
}

Scene::Scene()
{
	renderer_ = VkRenderer::getSingletonPtr();
	skybox_ = renderer_->addRenderSubsystem<SkyBoxRenderSubsystem>();
}

Scene::~Scene()
{
	registry_.clear();
}

BaseCamera* Scene::getCurrentCamera() const
{
	if (cameras_.size() > 0)
	{
		return  cameras_[currentCamera_];
	}
	else
	{
		return nullptr;
	}
}

void Scene::frameBufferResizedCallback(const Events::FrameBufferResizedEvent& e)
{
	for (BaseCamera* camera : cameras_)
	{
		camera->resetProjectionMatrix(e.width_, e.height_);
	}
}

void Scene::update(float deltaTime)
{
	auto v = registry_.view<Transform, Renderable>();
	for (auto entity : v)
	{
		auto& transform  = v.get<Transform>(entity);
		auto& mesh = v.get<Renderable>(entity);

		transform.drawDebugGui();

		transform.applyTransformToMesh(mesh);

		if (cameras_.size() != 0)
		{
			mesh.ubo_.view = cameras_[currentCamera_]->getView();
			mesh.ubo_.proj = cameras_[currentCamera_]->projectionMatrix;
		}

		auto animator = registry_.try_get<Animator>(entity); 
		if (animator && mesh.render_)
		{
			animator->update(deltaTime, mesh);
		}

		if (!mesh.uploaded_)
		{
			Textured* texture = registry_.try_get<Textured>(entity);
			if (texture != 0)
				renderer_->uploadObject(&mesh, texture, animator);
			else
				renderer_->uploadObject(&mesh);

			mesh.uploaded_ = true;
		}
		if(mesh.render_)
			objectsToDraw_.push_back(&mesh);
	}
	renderer_->draw(objectsToDraw_);
	objectsToDraw_.clear();
}

Entity* Scene::addEntity(std::string name)
{
	Entity* e = new Entity(registry_, name);
	entitys_.push_back(e);
	return e;
}

int Scene::addCamera(BaseCamera* camera)
{
	cameras_.push_back(camera);
	int cameraIndex = cameras_.size() - 1;
	camera->cameraIndex_ = cameraIndex;
	return cameraIndex;
}

void Scene::setCamera(int index)
{
	currentCamera_ = index;
}

bool Scene::setSkybox(const std::string& path)
{
	return(skybox_->setSkyboxTexture(path));
}

