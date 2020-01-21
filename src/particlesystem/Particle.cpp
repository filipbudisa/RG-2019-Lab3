#include "Particle.h"
#include "../storage/Storage.h"

Particle::Particle(WorldObject* wObject, glm::vec3 velocity, glm::vec3 force, double lifeTime) : wObject(wObject), velocity(velocity),
										force(force), lifeTime(lifeTime){

	rotation = ((double) rand() / RAND_MAX) * 2 * M_PI;
}

void Particle::update(double time){
	velocity += force * (float) time;
	wObject->position += velocity;
	wObject->setTransformation();

	lifeTime -= time;
}

double Particle::getLifeTime() const{
	return lifeTime;
}


WorldObject *Particle::getWObject() const{
	return wObject;
}

double Particle::getRotation() const{
	return rotation;
}
