#include "WorldObject.h"
#include "../storage/Storage.h"

WorldObject::WorldObject(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
		: position(position), rotation(rotation), scale(scale){

	Storage::worldObjects.push_back(this);
}


void WorldObject::setTransformation(){
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);

	/*glm::mat4 matPitch = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 matRoll  = glm::rotate(glm::mat4(1.0f), rotation.y,  glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 matYaw = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 rotate = matYaw * matPitch * matRoll;*/

	glm::mat4 scal = glm::scale(glm::mat4(1.0f), scale);

	/*glm::vec3 s = rotation * UP;
	glm::vec3 un = rotation * s;
	rotate = glm::mat4(glm::vec4(rotation, 0.0), glm::vec4(un, 0.0), glm::vec4(s, 0.0), { 0.0, 0.0, 0.0, 1.0 });
	rotate = glm::transpose(rotate);*/

	/*glm::vec3 rot = glm::vec3(rotation[0], rotation[1], rotation[2]);
	if(glm::length(rot) > 1e-6){
		rot = glm::normalize(rot);
		rotate = glm::lookAt(glm::vec3(0.0f), rot, UP);
	}else{
		rotate = glm::mat4(1.0);
	}*/

	//rotate = glm::mat4(1.0);

	glm::mat4 rotate = glm::toMat4(rotation);

	renderComponent->transforms.tObject = translation * rotate * scal;
	renderComponent->transforms.tNormal = rotate;
}

void WorldObject::update(double time){
 	for(IObjectModifier *modifier : modifiers){
		modifier->update(time);
	}
}

void WorldObject::addModifier(IObjectModifier *modifier){
	modifiers.push_back(modifier);
}

void WorldObject::setRender(RenderComponent *renderComponent){
	WorldObject::renderComponent = renderComponent;
	setTransformation();
}

void WorldObject::setCollision(CollisionComponent *collisionComponent){
	WorldObject::collisionComponent = collisionComponent;
}

WorldObject::WorldObject() : position({ 0, 0, 0 }), rotation({ 0, 0, 0 }), scale({ 1, 1, 1 }){
	Storage::worldObjects.push_back(this);
}

void WorldObject::setPhysics(IPhysicsComponent *physicsComponent){
	WorldObject::physicsComponent = physicsComponent;
}

void WorldObject::cleanup(){
	for(IObjectModifier* modifier : modifiers){
		delete modifier;
	}
}
