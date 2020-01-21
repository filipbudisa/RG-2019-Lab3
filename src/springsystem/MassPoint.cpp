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
	force += glm::vec3( 0, 0, -9.81 );

	// velocity
	glm::vec3 vl = velocity;
	if(glm::length(velLast) < 0.001) velLast = velocity;
	velocity = velocity + force * (float) time;
	velLast = vl;

	// position
	glm::vec3 pl = vertex->pos;
	if(glm::length(posLast) < 0.001) posLast = vertex->pos;
	vertex->pos = vertex->pos + velocity * (float) time ; //+ force * (float) pow(time, 2) / 2.0f;
	posLast = pl;
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
