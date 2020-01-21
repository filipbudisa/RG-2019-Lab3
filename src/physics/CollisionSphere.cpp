#include <glm/geometric.hpp>
#include "CollisionSphere.h"

glm::vec3 CollisionSphere::collision(glm::vec3 test, glm::vec3 pos){
	glm::vec3 diff = test - pos;
	float length = glm::length(diff);

	if(length < r){
		return diff * (r - length) / r;
	}

	return { 0, 0, 0 };
}

CollisionSphere::CollisionSphere(float r) : r(r){}
