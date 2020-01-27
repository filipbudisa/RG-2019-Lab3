#include "World.h"
#include "../storage/Storage.h"
#include "../utils.h"
#include "../Game.h"
#include "../curves/CosLine.h"
#include "../physics/CollisionSphere.h"
#include "../data.h"

void World::load(int scene){
	Mesh GroundPlane = Mesh({
			   {{-5.0f, -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{5.0f,  -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{5.0f,  5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{-5.0f, 5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}
	   }, { 0, 1, 2, 2, 3, 0 });

	WorldObject* gpObject = new WorldObject();
	gpObject->setRender(new RenderComponent(GroundPlane));

	switch(scene){
		case 1:
			load1();
			break;
		case 2:
			load2();
			break;
		case 3:
			load3();
			break;
	}

	updateTransformationmatrices();
}

void World::load1(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, noPoints);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, noPoints, 20);
	system->setFixed(0, 0, true);
	system->setFixed(noPoints-1, 0, true);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.29, 20, 20);
	WorldObject* ballObj = new WorldObject({ 0.5, -0.6, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));
	ballObj->addModifier(new CosLine(ballObj, { 0, -1, 0 }, 1, 8));

	mesh = Mesh::generateSphere(0.29, 20, 20);
	ballObj = new WorldObject({ -0.5, 0.6, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));
	ballObj->addModifier(new CosLine(ballObj, { 0, 1, 0 }, 1, 8));
}

void World::load2(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, noPoints);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, noPoints, 20);
	system->setFixed(0, 0, true);
	system->setFixed(noPoints-1, 0, true);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.49, 20, 20);
	WorldObject* ballObj = new WorldObject({ 0.0, 0.7, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.5)));
	ballObj->addModifier(new CosLine(ballObj, { 0, 1, 0 }, 1, 5));
}

void World::load3(){
	Mesh plane = Mesh::generatePlane({ -2, -2 }, { 2, 2 }, noPoints, true);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));
	planeObj->position = { 0, 0, 3.0 };

	SpringSystem* system = new SpringSystem(planeObj, noPoints, 20);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.29, 20, 20);
	WorldObject* ballObj = new WorldObject({ 1, 1, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));

	mesh = Mesh::generateSphere(0.29, 20, 20);
	ballObj = new WorldObject({ 1, -1, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));

	mesh = Mesh::generateSphere(0.29, 20, 20);
	ballObj = new WorldObject({ -1, 1, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));

	mesh = Mesh::generateSphere(0.29, 20, 20);
	ballObj = new WorldObject({ -1, -1, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.3)));
}

void World::cleanup(){
	Storage::clearGarbage();
	PhysicsEngine::cleanup();
	Storage::cleanup();
}

WorldObject* World::loadModel(const Mesh mesh, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale){
	RenderComponent *rObj = new RenderComponent(mesh);
	WorldObject *wObj = new WorldObject(translation, rotation, scale);
	wObj->setRender(rObj);

	Storage::renderObjects.push_back(rObj);
	Storage::worldObjects.push_back(wObj);

	return wObj;
}

void World::loadModel(const Mesh mesh){
	loadModel(mesh, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
}

float i = 0;
bool added = false;

void World::update(double time){
	updateTransformationmatrices();
}

void World::updateTransformationmatrices(){
	for(int i = 0; i < Storage::worldObjects.size(); i++){
		Storage::worldObjects[i]->setTransformation();
	}
}

World::World(){

}
