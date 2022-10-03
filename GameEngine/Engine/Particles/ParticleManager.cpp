#include "../EngineSettings.h"
#include "ParticleManager.h"
#include "../Scene/Scene.h"
#include "../ResourceManager.h"
#include "../Scene/Components/Camera.h"
#include "../Scene/Components/LightSource.h"

template<> ParticleManager* Singleton<ParticleManager>::msSingleton = nullptr;

ParticleManager::ParticleManager() : 
	Singleton<ParticleManager>()
{
	//initalize rng
	std::random_device randomDevice;
	rng_ = std::mt19937(randomDevice());
}

ParticleManager::~ParticleManager()
{}

ParticleManager& ParticleManager::getSingleton()
{
	assert(msSingleton);
	return *msSingleton;
}

ParticleManager * ParticleManager::getSingletonPtr()
{
	assert(msSingleton);
	return msSingleton;
}

int ParticleManager::createParticleEmitter(const CreateParticleEmitter& createParticleEmitter)
{
	int nextAvailableIndex = -1;
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++)
	{
		if (!particleEmitters_[i].active)
		{
			particleEmitters_[i].angularVel = UniformDist(createParticleEmitter.angularVel.min, createParticleEmitter.angularVel.max);
			particleEmitters_[i].lifeTime = UniformDist(createParticleEmitter.lifeTime.min, createParticleEmitter.lifeTime.max);
			particleEmitters_[i].scale = UniformDist(createParticleEmitter.scale.min, createParticleEmitter.scale.max);
			particleEmitters_[i].velocity = UniformDist(createParticleEmitter.velocity.min, createParticleEmitter.velocity.max);
			particleEmitters_[i].active = true;
			particleEmitters_[i].rate = createParticleEmitter.rate;
			particleEmitterTime_[i] = 0.0f;
			nextAvailableIndex = i;
			break;
		}
	}
	return nextAvailableIndex;
}

void ParticleManager::destroyParticleEmitter(int particleEmitterIndex)
{
	particleEmitters_[particleEmitterIndex].active = false;
}

void ParticleManager::addParticle(CreateParticleInfo createParticleInfo)
{
	Particle particle;
	particle.degreesPerFrame = createParticleInfo.degreesPerFrame;
	particle.active = true;
	particle.lifeTime = createParticleInfo.lifeTime;
	particle.size = createParticleInfo.size;
	particle.velocity = createParticleInfo.velocity;
	particle.entity = Scene::getSingleton().addEntity("Particle");
	AssetCreateInfo assetCreateInfo;
	assetCreateInfo.texturePath = createParticleInfo.texturePath;
	assetCreateInfo.modelPath = QUAD_PATH;
	Asset* asset = ResourceManager::getSingleton().createAsset(assetCreateInfo);
	PipelineCreateInfo createInfo;
	createInfo.windowExtent.width = EngineSettings::getSingleton().windowWidth;
	createInfo.windowExtent.height = EngineSettings::getSingleton().windowHeight;
	createInfo.cullingEnabled = true;
	createInfo.depthEnabled = true;
	createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.vertexShader = RendererInterface::getSingleton().getParticleVertexShader(createParticleInfo.vertexType);
	createInfo.fragmentShader = RendererInterface::getSingleton().getParticleFragmentShader(createParticleInfo.fragmentType);
	createInfo.hasSkeleton = false;
	particle.entity->addComponent<AssetInstance>(asset, createInfo);
	Transform& transform = particle.entity->addComponent<Transform>(createParticleInfo.startingPos);
	transform.setScale(particle.size);
	if (createParticleInfo.lighting)
	{
		PointLight& pointLight = particle.entity->addComponent<PointLight>();
		pointLight.uniformData_.ambient = { 0.00f, 0.00f, 0.00f };
		pointLight.uniformData_.diffuse = { 0.05f, 0.05f, 0.05f };
		pointLight.uniformData_.specular = { 1.0f, 1.0f, 1.0f };
	}
	if (createParticleInfo.applyGravity)
	{
		particle.acceleration = { 0.0f, -0.15f ,0.0f };
	}
	int index = findNextOpenParticle();
	_ASSERT(index != -1);
	particles_[index] = particle;
}

void ParticleManager::update(float dt)
{
	dt *= 0.001f;
	updateParticleEmitters(dt);
	Camera& camera = Scene::getSingleton().getCurrentCamera();
	for (int i = 0; i < MAX_PARTICLES; i++)
	{		
		Particle& particle = particles_[i];
		if (!particle.active)
		{
			continue;
		}
		Transform& transform = particle.entity->getComponent<Transform>();
		glm::vec3 cameraPos = camera.entity_->getComponent<Transform>().position_;
		transform.lookAt(cameraPos - transform.position_);
		particle.velocity += particle.acceleration;
		transform.position_ += particle.velocity * dt;
		particle.currentRotation += particle.degreesPerFrame;
		transform.rotateAround(particle.currentRotation, Transform::worldForward);
		particle.lifeTime -= dt;
		if (particle.lifeTime <= 0.0f)
		{
			RendererInterface::getSingleton().deleteDynamicAssetData(particle.entity);
			particle = {};
		}
	}
}

int ParticleManager::findNextOpenParticle()
{
	int index = -1;
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (!particles_[i].active)
		{
			index = i;
			break;
		}
	}
	return index;
}

void ParticleManager::updateParticleEmitters(float dt)
{
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++)
	{
		ParticleEmitter* emitter = &particleEmitters_[i];
		if (emitter->active)
		{
			if (particleEmitterTime_[i] < 1.0f)
			{
				particleEmitterTime_[i] += dt;
				continue;
			}

			for (int particleIndex = 0; particleIndex < emitter->rate; particleIndex++)
			{
				CreateParticleInfo createParticleInfo;
				createParticleInfo.degreesPerFrame = emitter->angularVel(rng_);
				createParticleInfo.lifeTime = emitter->lifeTime(rng_);
				createParticleInfo.size = emitter->scale(rng_);
				createParticleInfo.startingPos = emitter->position;
				createParticleInfo.texturePath = emitter->texture;
				createParticleInfo.velocity = {emitter->velocity(rng_), emitter->velocity(rng_), emitter->velocity(rng_) };
				createParticleInfo.velocity *= dt;
				addParticle(createParticleInfo);
			}
			particleEmitterTime_[i] = 0.0f;
		}
	}
}
