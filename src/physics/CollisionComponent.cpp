#include "CollisionComponent.h"
#include "PhysicsEngine.h"

CollisionComponent::CollisionComponent(WorldObject *object, ICollisionObject *collisionObject) : object(object),
																								 collisionObject(
																										 collisionObject){
	PhysicsEngine::colComps.push_back(this);
}

glm::vec3 CollisionComponent::collide(glm::vec3 test){
	return collisionObject->collision(test, object->position);
}

WorldObject *CollisionComponent::getObject() const{
	return object;
}

void CollisionComponent::cleanup(){
	delete collisionObject;
}
