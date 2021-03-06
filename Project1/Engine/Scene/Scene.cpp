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
	auto v = registry_.view<Transform, Renderable>();
	for (auto entity : v)
	{
		auto [transform, mesh] = v.get<Transform, Renderable>(entity);

		transform.drawDebugGui();

		transform.applyTransformToMesh(mesh);

		if (cameras_.size() != 0)
			mesh.ubo().view = cameras_[currentCamera_]->getView();
		

		auto[animator, bones] = registry_.try_get<Animator, BoneStructure>(entity); 
		if (animator && bones)
		{
			std::vector<aiMatrix4x4> boneTransforms = animator->update(deltaTime, *bones);
			bones->setPose(boneTransforms, mesh);
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
