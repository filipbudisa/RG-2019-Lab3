#ifndef VULK_PARTICLE_H
#define VULK_PARTICLE_H


#include "../interfaces/ITimeBound.h"
#include "../graphics/RenderComponent.h"
#include "../world/WorldObject.h"

class Particle : public ITimeBound {
public:
	Particle(WorldObject* wObject, glm::vec3 velocity, glm::vec3 force, double lifeTime);

	void update(double time) override;

	double getLifeTime() const;

	WorldObject *getWObject() const;

	double getRotation() const;

private:
	glm::vec3 velocity;
	glm::vec3 force;

	double lifeTime;

	WorldObject* wObject;
	double rotation;
};


#endif //VULK_PARTICLE_H
