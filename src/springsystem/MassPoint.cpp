#include <cmath>
#include <glm/glm.hpp>
#include "MassPoint.h"

MassPoint::MassPoint(Vertex* vertex, MeshTransforms* transforms, float mass) : vertex(vertex), transforms(transforms), mass(mass){}

void MassPoint::resetForce(){
	//force = { 0, 0, -9.81 };
	force = { 0, 0, 0 };
	//velocity = { 0, 0, 0 };
}

void MassPoint::addForce(glm::vec3 force){
	this->force += force;
}

void MassPoint::addVelocity(glm::vec3 velocity){
	this->velocity += velocity;
}

void MassPoint::update(double time){
	if(fixed) return;

	// damping
	force += velocity * -3.0f;

	// gravity
	force += glm::vec3( 0, 0, -9.81 );

	// velocity
	velocity = velocity + force * (float) time;

	// position
	vertex->pos = vertex->pos + velocity * (float) time ;
}

glm::vec3 MassPoint::getPosition() const{
	return vertex->pos;
}

void MassPoint::setFixed(bool fixed){
	this->fixed = fixed;
}

void MassPoint::setPosition(const glm::vec3& position){
	vertex->pos = position;
}

void MassPoint::move(const glm::vec3& direction){
	vertex->pos += direction;
}

const glm::vec3& MassPoint::getVelocity() const{
	return velocity;
}

float MassPoint::getMass() const{
	return mass;
}
