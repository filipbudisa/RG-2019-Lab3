#include "PhysicsEngine.h"
#include "../Game.h"
#include "../storage/Storage.h"

std::vector<CollisionComponent*> PhysicsEngine::colComps;
std::vector<IPhysicsComponent*> PhysicsEngine::physComps;

void PhysicsEngine::update(double time){
	time += timeResidue;

	while((time - TIME_DELTA) > 0){
		for(IPhysicsComponent* physComp : physComps){
			physComp->resetForce();
		}

		for(WorldObject *obj : Storage::worldObjects){
			obj->update(TIME_DELTA);
		}

		for(int i = 0; i < physComps.size(); i++){
			for(int j = 0; j < colComps.size(); j++){
				physComps[i]->collide(colComps[j]);
			}
		}

		for(IPhysicsComponent* physComp : physComps){
			physComp->update(TIME_DELTA);
		}

		time -= TIME_DELTA;
	}

	timeResidue = time;
}

void PhysicsEngine::cleanup(){
	for(CollisionComponent* c : colComps){
		c->cleanup();
		delete c;
	}

	for(IPhysicsComponent* c : physComps){
		delete c;
	}

	colComps.clear();
	physComps.clear();
}
