#include "Scene.h"

Scene::Scene()
{
	renderer_ = VkRenderer::getSingletonPtr();


}

Scene::~Scene()
{
	registry_.clear();
}

void Scene::update(float deltaTime)
{
	auto v = registry_.view<Transform, Renderable, Textured>();
	for (auto entity : v)
	{
		auto [transform, mesh, texture] = v.get<Transform, Renderable, Textured>(entity);

		transform.drawDebugGui(true);

		transform.applyTransformToMesh(mesh);

		if (cameras_.size() != 0)
			mesh.ubo_.view = cameras_[currentCamera_]->getView();
		
		if (!mesh.uploaded_)
		{
			renderer_->uploadObject(&mesh, &texture);
			mesh.uploaded_ = true;
		}
	}
}

Entity& Scene::addEntity(std::string name)
{
	Entity* e = new Entity(registry_, name);
	entitys_.push_back(e);
	return *e;
}

int Scene::addCamera(BaseCamera* camera)
{
	cameras_.push_back(camera);
	return cameras_.size() - 1;
}
