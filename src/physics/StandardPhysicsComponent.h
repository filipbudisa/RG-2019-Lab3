#ifndef VULK_STANDARDPHYSICSCOMPONENT_H
#define VULK_STANDARDPHYSICSCOMPONENT_H


#include <glm/vec3.hpp>
#include "../interfaces/ITimeBound.h"
#include "../world/WorldObject.h"
#include "IPhysicsComponent.h"

class StandardPhysicsComponent : public IPhysicsComponent {
public:
	StandardPhysicsComponent(WorldObject *object);

	void update(double time) override;

	void resetForce() override;
	void addForce(glm::vec3 force);
	void addVelocity(glm::vec3 velocity);

private:
	WorldObject* object;

	glm::vec3 posLast = { 0, 0, 0 };
	glm::vec3 velocity = { 0, 0, 0 };
	glm::vec3 force;
	bool fixed = false;
};


#endif //VULK_STANDARDPHYSICSCOMPONENT_H
