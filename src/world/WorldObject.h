#ifndef VULK_WORLDOBJECT_H
#define VULK_WORLDOBJECT_H


#include "../graphics/RenderComponent.h"
#include "../interfaces/ITimeBound.h"
#include "../interfaces/IObjectModifier.h"
#include "../physics/CollisionComponent.h"
#include "../physics/IPhysicsComponent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class IObjectModifier;
class CollisionComponent;
class IPhysicsComponent;

class WorldObject : public ITimeBound {
public:
	WorldObject();

	WorldObject(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale);

	void addModifier(IObjectModifier* modifier);
	void setTransformation();
	virtual void update(double time);
	void cleanup();

	void setRender(RenderComponent *renderComponent);
	void setCollision(CollisionComponent *collisionComponent);
	void setPhysics(IPhysicsComponent *physicsComponent);

	RenderComponent* renderComponent = nullptr;
	CollisionComponent* collisionComponent = nullptr;
	IPhysicsComponent* physicsComponent = nullptr;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

private:
	std::vector<IObjectModifier*> modifiers;
};


#endif //VULK_WORLDOBJECT_H
