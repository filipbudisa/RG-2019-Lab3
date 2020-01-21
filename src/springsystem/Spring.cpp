#include "Spring.h"

Spring::Spring(uint16_t a, uint16_t b, float k) : k(k), pointsIndexes(a, b){

}

void Spring::update(double time){
	glm::vec3 posa = points.first->getPosition() * system->object->scale;
	glm::vec3 posb = points.second->getPosition() * system->object->scale;

	glm::vec3 direction = posa - posb;
	//direction = glm::cross(direction, system->object->scale);

	double currentLength = glm::length(direction);
	double force = -k * (currentLength - length);

	direction = glm::normalize(direction);

	points.first->addForce(direction * (float) (force / 2.0));
	points.second->addForce(direction * (float) (force / -2.0));

	glPoints.first->force += direction * (float) (force / 2.0);
	glPoints.second->force += direction * (float) (force / -2.0);

}

void Spring::updateVertices(){
	vertices[0].pos = points.first->getPosition() * system->object->scale + system->object->position;
	vertices[1].pos = points.second->getPosition() * system->object->scale + system->object->position;
}

void Spring::setSystem(SpringSystem *system){
	Spring::system = system;

	MassPoint* a = system->getPoint(pointsIndexes.first);
	MassPoint* b = system->getPoint(pointsIndexes.second);

	points = { a, b };

	glPoints = { &system->glPoints[pointsIndexes.first], &system->glPoints[pointsIndexes.second] };

	glm::vec3 posa = a->getPosition() * system->object->scale + system->object->position;
	glm::vec3 posb = b->getPosition() * system->object->scale + system->object->position;

	Vertex va = { posa };
	Vertex vb = { posb };

	va.color.b = vb.color.b = 1.0;

	vertices.push_back(va);
	vertices.push_back(vb);

	length = glm::length(a->getPosition() - b->getPosition());
}

const std::pair<uint16_t, uint16_t>& Spring::getIndexes() const{
	return pointsIndexes;
}
