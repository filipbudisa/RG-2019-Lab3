#ifndef VULK_PARTICLESYSTEM_H
#define VULK_PARTICLESYSTEM_H


#include <glm/vec3.hpp>
#include "../interfaces/ITimeBound.h"
#include "Particle.h"

class ParticleSystem : public ITimeBound {
public:
	ParticleSystem(const glm::vec3& source, const glm::vec3& direction, double directionVariance,
				   double initialVelocity, const glm::vec3& particleForce, double spawnInterval,
				   double particleLifeTime);

	void update(double time) override;

	void setSource(const glm::vec3& source);

private:
	Particle* createParticle();

	glm::vec3 source;
	glm::vec3 direction;
	float directionVariance;
	float initialVelocity;
	glm::vec3 particleForce;
	double spawnInterval; // in ms
	double particleLifeTime;

	std::vector<Particle*> particles;

	double timeResidue = 0;
	double spawnTimer = 0;
};


#endif //VULK_PARTICLESYSTEM_H
