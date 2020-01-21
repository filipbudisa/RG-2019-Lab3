#ifndef VULK_COLLISIONSPHERE_H
#define VULK_COLLISIONSPHERE_H


#include <glm/vec3.hpp>
#include "ICollisionObject.h"

class CollisionSphere : public ICollisionObject {
public:
	CollisionSphere(float r);

	virtual glm::vec3 collision(glm::vec3 test, glm::vec3 pos);

private:
	float r;
};


#endif //VULK_COLLISIONSPHERE_H
