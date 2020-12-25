#include "Scene.h"

Scene::~Scene()
{
	registry_.clear();
}

void Scene::update(float deltaTime)
{
	auto v = registry_.view<Transform, Mesh>();
	for (auto entity : v)
	{
		auto [t, mesh] = v.get<Transform, Mesh>(entity);

		t.applyTransformToMesh(mesh);

		if (cameras_.size() != 0)
			mesh.m_ubo.view = cameras_[currentCamera_].getView();

		mesh.draw();

	}
}

Entity& Scene::addEntity(std::string name)
{
	Entity* e = new Entity(registry_, name);
	entitys_.push_back(e);
	return *e;
}

