#include "Scene.h"

Scene::Scene()
{
	renderer_ = VkRenderer::getSingletonPtr();
}

Scene::~Scene()
{
	registry_.clear();
}

BaseCamera* Scene::getCurrentCamera()
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
			mesh.ubo_.view = cameras_[currentCamera_]->getView();
		

		auto animator = registry_.try_get<Animator>(entity); 
		if (animator)
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
	return cameras_.size() - 1;
}

void Scene::setCamera(int index)
{
	currentCamera_ = index;
}
