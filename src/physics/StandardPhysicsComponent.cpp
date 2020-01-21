#include <cmath>
#include "StandardPhysicsComponent.h"

void StandardPhysicsComponent::update(double time){
	if(fixed) return;

	//force += velocity * -0.1f; // damping

	velocity += force * (float) pow(time, 1);
	//position += velocity * (float) pow(time, 1);

	glm::vec3 pl = object->position;
	if(glm::length(posLast) < 0.01) posLast = object->position;
	object->position = object->position * 2.0f - posLast + force * (float) pow(time, 2);
	posLast = pl;
}

void StandardPhysicsComponent::resetForce(){
	force = { 0, 0, -0.981 };
}

void StandardPhysicsComponent::addForce(glm::vec3 force){
	this->force += force;
}

void StandardPhysicsComponent::addVelocity(glm::vec3 velocity){
	this->velocity += velocity;
}

StandardPhysicsComponent::StandardPhysicsComponent(WorldObject *object) : object(object){}
