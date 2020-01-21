#ifndef VULK_ICOLLISIONOBJECT_H
#define VULK_ICOLLISIONOBJECT_H


#include <glm/vec3.hpp>

class ICollisionObject {
public:
	virtual glm::vec3 collision(glm::vec3 test, glm::vec3 pos) = 0;
};


#endif //VULK_ICOLLISIONOBJECT_H
