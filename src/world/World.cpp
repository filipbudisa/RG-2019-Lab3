#include "World.h"
#include "../graphics/RenderComponent.h"
#include "../storage/Storage.h"
#include "../utils.h"
#include "../curves/Bspline.h"
#include "../particlesystem/ParticleSystem.h"
#include "../springsystem/MassPoint.h"
#include "../springsystem/SpringSystem.h"
#include "../Game.h"
#include "../curves/CosLine.h"
#include "../physics/CollisionSphere.h"

glm::vec3 rot;
glm::vec3 pos;

WorldObject* teddy;

void World::load(int i){
	/*RenderComponent *rObj = new RenderComponent(Mesh({
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f,  -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f,  0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f},  {1.0f, 1.0f, 1.0f}}
		},
		{ 0, 1, 2, 2, 3, 0 }));

	Storage::renderObjects.push_back(rObj);

	Storage::worldObjects.push_back(new WorldObject(rObj, { 3.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
	Storage::worldObjects.push_back(new WorldObject(rObj, { -3.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
	Storage::worldObjects.push_back(new WorldObject(rObj, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));*/

	Mesh GroundPlane = Mesh({
			   {{-5.0f, -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{5.0f,  -5.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{5.0f,  5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
			   {{-5.0f, 5.0f, 0.0f},  {0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}
	   }, { 0, 1, 2, 2, 3, 0 });

	WorldObject* gpObject = new WorldObject();
	gpObject->setRender(new RenderComponent(GroundPlane));

	// loadModel(readObj("pillar.obj"), glm::vec3(3.0f, 3.0f, 0.0f), glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	// loadModel(readObj("pillar.obj"), glm::vec3(3.0f, -3.0f, 0.0f), glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	// loadModel(readObj("pillar.obj"), glm::vec3(-3.0f, -3.0f, 0.0f), glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	// loadModel(readObj("pillar.obj"), glm::vec3(-3.0f, 3.0f, 0.0f), glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));

	//loadModel(readObj("monkey.obj"), { 0, -1.5f, 0.0f }, { glm::half_pi<float>(), 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
	//loadModel(readObj("teddy.obj"), { 0.0f, 1.5f, 1.0f }, { glm::half_pi<float>(), 0.0f, 0.0f }, { 0.05f, 0.05f, 0.05f });

	/*teddy = loadModel(readObj("teddy.obj"), { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.03f, 0.03f, 0.03f });
	Bspline* spline = new Bspline(teddy, "spline.txt", 10);
	teddy->addModifier(spline);
	Storage::splines.push_back(spline);*/

	//Bspline* spline = new Bspline( "spline.txt", 10);
	//Storage::splines.push_back(spline);

	/*pSystems.push_back(ParticleSystem(
			{ -2, -2, 1 },
			{ 0, 0, 1 },
			0.8,
			0.02,
			{ 0, 0, -.0098 },
			100,
			4));

	pSystems.push_back(ParticleSystem(
			{ 0, 0, 0 },
			{ 0, 0, 1 },
			1,
			0.005,
			{ 0, 0, -.0098 },
			20,
			2)); */

	scene = i;

	switch(i){
		case 1:
			load1();
			break;
		case 2:
			load2();
			break;
		case 3:
			load3();
			break;
		case 4:
			load4();
			break;
		case 5:
			load5();
			break;
		case 6:
			load6();
			break;
	}

	updateTransformationmatrices();
}

void World::load1(){
	Mesh plane = Mesh::generatePlane({ 0, 0 }, { 2, 2 }, 20, true);
	WorldObject* planeObj = new WorldObject();
	planeObj->position = { 0, 0, 3 };
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, 20, 20);
	system->setFixed(19, 0, true);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.49, 20, 20);
	WorldObject* ballObj = new WorldObject({ 1.0, 1.0, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });
	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.5)));
}

void World::load2(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, 20);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, 20, 20);
	system->setFixed(0, 0, true);
	system->setFixed(19, 0, true);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.49, 20, 20);
	WorldObject* ballObj = new WorldObject({ 0.0, 1.0, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.5)));
	ballObj->addModifier(new CosLine(ballObj, { 0, 1, 0 }, 1, 5));
}

void World::load3(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, 20);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, 20, 20);
	system->setFixed(0, 0, true);
	system->setFixed(19, 0, true);
	planeObj->setPhysics(system);

	Mesh mesh = Mesh::generateSphere(0.49, 20, 20);
	WorldObject* ballObj = new WorldObject({ 0.0, 0.6, 2.0 }, { 0.0, 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 });	ballObj->setRender(new RenderComponent(mesh));
	ballObj->setCollision(new CollisionComponent(ballObj, new CollisionSphere(0.5)));
	ballObj->addModifier(new CosLine(ballObj, { 0, 1, 0 }, 2, 5));
}

void World::load4(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, 20);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, 20, 20);
	system->setFixed(0, 0, true);
	system->setFixed(19, 0, true);
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

void World::load5(){
	Mesh plane = Mesh::generatePlane({ 2, 2 }, { -2, -2 }, 15, true);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));
	planeObj->position = { 0, 0, 3 };

	SpringSystem* system = new SpringSystem(planeObj, 15, 20);
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

void World::load6(){
	Mesh plane = Mesh::generatePlane({ 1, 3 }, { -1, 1 }, 10);
	WorldObject* planeObj = new WorldObject();
	planeObj->setRender(new RenderComponent(plane));

	SpringSystem* system = new SpringSystem(planeObj, 10, 20);
	system->setFixed(0, 0, true);
	system->setFixed(9, 0, true);
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
	for(WorldObject *obj : Storage::worldObjects){
		//obj->update(time);
	}

	/*for(Bspline *s : Storage::splines){
		s->update(time);
	}

	pSystems[1].setSource(Storage::splines[0]->tVert * glm::vec4({ 0.0, 0.0, 1.0, 1.0 }));

	for(ParticleSystem& p : pSystems){
		p.update(time);
	}*/

	updateTransformationmatrices();
}

void World::updateTransformationmatrices(){
	for(int i = 0; i < Storage::worldObjects.size(); i++){
		Storage::worldObjects[i]->setTransformation();
	}
}

World::World(){

}
