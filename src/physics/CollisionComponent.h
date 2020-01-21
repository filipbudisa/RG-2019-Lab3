#ifndef VULK_COLLISIONCOMPONENT_H
#define VULK_COLLISIONCOMPONENT_H


#include "ICollisionObject.h"
#include "../world/WorldObject.h"

class WorldObject;

class CollisionComponent {
public:
	CollisionComponent(WorldObject *object, ICollisionObject *collisionObject);

	glm::vec3 collide(glm::vec3 test);

	WorldObject *getObject() const;

	void cleanup();

private:
	WorldObject* object;
	ICollisionObject* collisionObject;
};


#endif //VULK_COLLISIONCOMPONENT_H
