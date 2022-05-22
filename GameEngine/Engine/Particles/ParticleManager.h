#pragma once

#include <random>
#include <list>
#include <map>

#include "../Singleton.h"
#include "ParticleTypes.h"
#include "../Vertex.h"
#include "../EngineExport.h"

class ENGINE_API ParticleManager : Singleton<ParticleManager>
{
public:

	ParticleManager();
	~ParticleManager();

	static ParticleManager& getSingleton();
	static ParticleManager* getSingletonPtr();

	int createParticleEmitter(const CreateParticleEmitter& createParticleEmitter);
	void destroyParticleEmitter(int particleEmitterIndex);
	void addParticle(CreateParticleInfo createParticleInfo);

	void update(float dt);
private:
	int findNextOpenParticle();
	void updateParticleEmitters(float dt);

#define MAX_PARTICLES 1000 
	Particle particles_[MAX_PARTICLES];
#define MAX_PARTICLE_EMITTERS 100
	ParticleEmitter particleEmitters_[MAX_PARTICLE_EMITTERS];
	float particleEmitterTime_[MAX_PARTICLE_EMITTERS];

	std::mt19937 rng_;

const std::string QUAD_PATH = "Models/quad.obj";
};
