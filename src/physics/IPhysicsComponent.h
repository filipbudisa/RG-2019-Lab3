#ifndef VULK_IPHYSICSCOMPONENT_H
#define VULK_IPHYSICSCOMPONENT_H


#include "../interfaces/ITimeBound.h"
#include "CollisionComponent.h"

class CollisionComponent;

class IPhysicsComponent : public ITimeBound {
public:
	virtual void resetForce() = 0;
	virtual void collide(CollisionComponent* collidor) = 0;
};


#endif //VULK_IPHYSICSCOMPONENT_H
