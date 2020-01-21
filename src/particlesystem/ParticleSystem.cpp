#include "ParticleSystem.h"
#include "../world/WorldObject.h"
#include "../storage/Storage.h"
#include "../Game.h"

ParticleSystem::ParticleSystem(const glm::vec3& source, const glm::vec3& direction, double directionVariance,
							   double initialVelocity, const glm::vec3& particleForce, double spawnInterval,
							   double particleLifeTime)
							   : source(source), direction(direction), directionVariance(directionVariance),
							   initialVelocity(initialVelocity), particleForce(particleForce),
							   spawnInterval(spawnInterval), particleLifeTime(particleLifeTime){


}

Particle* ParticleSystem::createParticle(){
	RenderComponent *rObj = new RenderComponent(Mesh({
			   {{-5.0f, -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{5.0f,  -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
			   {{5.0f,  5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},
			   {{-5.0f, 5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}
	   }, { 0, 1, 2, 2, 3, 0 }));
	WorldObject *wObj = new WorldObject(source, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.01f, 0.01f, 0.01f));
	wObj->setRender(rObj);

	Storage::worldObjects.push_back(wObj);

	glm::vec3 pDirection = direction;
	for(int i = 0; i < 3; i++) pDirection[i] += (((double) rand()/RAND_MAX) - 0.5) * directionVariance;
	glm::vec3 pVelocity = glm::normalize(pDirection) * initialVelocity;

	Particle* particle = new Particle(wObj, pVelocity, particleForce, particleLifeTime);

	return particle;
}

void ParticleSystem::update(double time){
	int toCreate = (int) (spawnTimer / spawnInterval);
	spawnTimer += time*1000.0;

	for(int i = 0; i < toCreate; i++){
		spawnTimer -= spawnInterval;
		particles.push_back(createParticle());
		Storage::addParticle(particles.back());
		printf("Creating particle\n");
	}

	time += timeResidue;

	while((time - TIME_DELTA) > 0){
		std::vector<unsigned> erasing;

		for(int i = 0; i < particles.size(); i++){
			Particle* p = particles[i];

			p->update(TIME_DELTA);

			if(p->getLifeTime() < 0){
				erasing.push_back(i);
			}
		}

		for(int i = erasing.size()-1; i >= 0; i--){
			Particle* p = particles[erasing[i]];

			Storage::removeParticle(p);

			particles.erase(particles.begin() + erasing[i]);
		}

		time -= TIME_DELTA;
	}

	timeResidue = time;
}

void ParticleSystem::setSource(const glm::vec3& source){
	ParticleSystem::source = source;
}
